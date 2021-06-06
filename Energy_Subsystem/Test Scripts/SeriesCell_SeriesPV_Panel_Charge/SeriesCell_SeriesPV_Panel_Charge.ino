// Charging a single cell with a PV panel
  // No Discharging
// Port A: PV Panel, voltage available at A0
// Port B: A single battery
// Aim: To maximise solar panel output while ensuring that the current is delievered at a safe level.
  // Instead of using a reference voltage, alter PWM instead
  // Asynchronous Buck

#define PIN_VA A0

#define PIN_OLCL 2
#define PIN_PWM 6
#define PIN_REDLED 7
#define PIN_YELLED 8
#define PIN_V1 A1
#define PIN_V2 A2
#define PIN_V3 A3
#define PIN_DISC1 10
#define PIN_DISC2 A7
#define PIN_DISC3 A6
#define PIN_RLY1 5
#define PIN_RLY2 4
#define PIN_RLY3 9

#define IDLE 0
#define CHARGE 1
#define CHARGE_REST 2
#define SLOW_DISCHARGE 3
#define DISCHARGE_REST 4
#define ERROR 5
#define CV_CHARGE 6
#define RECAL_DONE 7
#define DISCHARGE 8
#define RAPID_DISCHARGE 9
#define RAPID_CHARGE 10

#include <Wire.h>
#include <INA219_WE.h>
#include <SPI.h>
#include <SD.h>
#include <MovingAverage.h>
#include "Power.h"

INA219_WE ina219; // this is the instantiation of the library for the current sensor
SMPS mySMPS;

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

const int SD_CS = 10;
unsigned int rest_timer;
unsigned int loop_trigger;
unsigned int int_count = 0; // a variables to count the interrupts. Used for program debugging.

// Current Controller Stuff
float u0i, u1i, delta_ui, e0i, e1i, e2i; // Internal values for the current controller
float ui_max = 1, ui_min = 0; //anti-windup limitation
float kpi = 0.02512, kii = 39.4, kdi = 0; // current pid.
float Ts = 0.001; //1 kHz control frequency.
float current_measure, current_ref = 0, error_amps; // Current Control
float pwm_out, closed_pwm;

// Voltage PID Controller Stuff
float ev=0,cv=0,ei=0; //internal signals // FIXME:
  // ev: difference between V_ref and V_b
  // cv: current obtained from voltage PID controller. need to saturate it
  // ei: difference between desired and inductor current. error_amps in this case? FIXME:
float kpv=0.05024,kiv=15.78,kdv=0; // voltage pid.
float u0v,u1v,delta_uv,e0v,e1v,e2v; // u->output; e->error; 0->this time; 1->last time; 2->last last time
float uv_max=4, uv_min=0; //anti-windup limitation

// Series Batteries Variables
float V_1 = 0, V_2 = 0, V_3 =0;
float SoC_1 = 0, SoC_2 = 0, SoC_3 = 0;
float V_UPLIM = 3590;
float V_LOWLIM = 2500;

float V_PD;
boolean input_switch;
int state_num=0,next_state, prev_state;
String dataString;

// PV Panel Limits
float vref;
float I_in;
float current_limit = 230;
float V_limit = 5000;
float v0, v1; // current and previous voltage values
float p0, p1; // current and previous power values
float i0, i1; // current and previous current values
float p_diff, v_diff;

// Current Capacity: Only calculated during discharge process
float q1 = 0, q2 = 0, q3 = 0;

// Stores the amount of charge added/removed within the past 2 minutes. Reset after.
float dq1 = 0, dq2 = 0, dq3 = 0;
bool disc1 = 0, disc2 = 0, disc3 = 0; // Account for difference in current when relay is on;
bool relay_on = 0;

void setup() {
  //Some General Setup Stuff

  Wire.begin(); // We need this for the i2c comms for the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  ina219.init(); // this initiates the current sensor
  Serial.begin(9600); // USB Communications

  mySMPS.init();

  if (SD.exists("PVCELLS.CSV")) {
      SD.remove("PVCELLS.CSV");
  }
 
  noInterrupts(); //disable all interrupts
  analogReference(EXTERNAL); // We are using an external analogue reference for the ADC

  //SMPS Pins
  pinMode(13, OUTPUT); // Using the LED on Pin D13 to indicate status
  pinMode(PIN_OLCL, INPUT_PULLUP); // Pin 2 is the input from the CL/OL switch
  pinMode(6, OUTPUT); // This is the PWM Pin

  //LEDs on pin 7 and 8
  pinMode(PIN_REDLED, OUTPUT);
  pinMode(PIN_YELLED, OUTPUT);

  //Input for measurements of battery 1,2,3
  pinMode(PIN_V1, INPUT);
  pinMode(PIN_V2, INPUT);
  pinMode(PIN_V3, INPUT);

  //Discharge for battery 1,2,3
  pinMode(PIN_DISC1, OUTPUT);
  pinMode(PIN_DISC2, OUTPUT);
  pinMode(PIN_DISC3, OUTPUT);

  //Relays for batteries 1,2,3
  pinMode(PIN_RLY1, OUTPUT);
  pinMode(PIN_RLY2, OUTPUT);
  pinMode(PIN_RLY3, OUTPUT);

  // Voltage reading at port A
  pinMode(PIN_VA, INPUT);

  // TimerA0 initialization for 200Hz control-loop interrupt.
  TCA0.SINGLE.PER = 4999; //
  TCA0.SINGLE.CMP1 = 4999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm;

  // TimerB0 initialization for PWM output
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz

  interrupts();  //enable interrupts.
  analogWrite(6, 120); //just a default state to start with

}

void loop() {
  if (loop_trigger == 1){ // FAST LOOP (200HZ)
      state_num = next_state; //state transition
      
      V_PD = analogRead(PIN_VA)*4.096/1.03* 4.1626; //mannual correction for potential divider
      
      //Checking for Error states (individual battery voltages defined)
      if ((V_1 > 3700 || V_1 < 2400) || (V_2 > 3700 || V_2 < 2400) || (V_3 > 3700 || V_3 < 2400)) { 
          state_num = ERROR; //go directly to jail
          next_state = ERROR; // stay in jail
          digitalWrite(PIN_REDLED,true); //turn on the red LED
          current_ref = 0; // no current
      }
      current_measure = (ina219.getCurrent_mA()); // sample the inductor current (via the sensor chip)
      
      //TODO: determine current in
      pwm_out = saturation(pwm_out, 0.99, 0.01);  //duty_cycle saturation
      analogWrite(6, (int)(255 - pwm_out * 255)); // write it out (inverting for the Buck here)
      int_count++; //count how many interrupts since this was last reset to zero
      loop_trigger = 0; //reset the trigger and move on with life
  }

  // Relay timer is reset every second. Like int_count, it also increments per millisecond.
  // Only switch on relay 1 time per second, and switch on them consecutively
  if (int_count == 100) { // Relay switching is 10ms. Double for safety
    digitalWrite(5,true);
  } else if (int_count == 104) { // Read battery 1 voltage
    V_1 = analogRead(A1)*4.096/1.03;
  } else if (int_count == 106) {
    digitalWrite(5,false);
  } else if (int_count == 110) {
    digitalWrite(4,true);
  } else if (int_count == 114) {
    V_2 = analogRead(A2)*4.096/1.03;
  } else if (int_count == 116) {
    digitalWrite(4,false);
  } else if (int_count == 120) {
    digitalWrite(9,true);
  } else if (int_count == 124) {
    V_3 = analogRead(A3)*4.096/1.03;
  } else if (int_count == 126) {
    digitalWrite(9,false);
    relay_on = 1;
  }
  
  if (int_count % 200 == 0) { // SLOW LOOP (1Hz)
    input_switch = digitalRead(PIN_OLCL); //get the OL/CL switch status
    switch (state_num) { // STATE MACHINE (see diagram)
      case 0:{ // Start state (no current, no LEDs)
        current_ref = 0;
        q1 = 0; q2 = 0; q3 = 0;
        pwm_out = 0;
        if (input_switch == 1) { // if switch, move to charge
          // First time, so reset voltage panel values
          pwm_out = 0.5; // initial pwm value
          
          v1 = V_PD;
          i1 = current_measure;
          p1 = v1*i1;
          pwm_out = pwm_out + 0.03;

          next_state = 1;
          digitalWrite(PIN_YELLED,true);
        } else { // otherwise stay put
          next_state = 0;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case 1:{ // Charge state (250mA and a green LED)

        // Assign values
        v0 = V_PD;
        i0 = current_measure;
        
        p0 = v0 * i0; // directly use I_out as a proxy indicator for I_in
        p_diff = p0-p1;  
        v_diff = v0-v1;

        // in general increasing PWM means decreasing PV voltage

        // PnO algorithm
        if (((p0>p1) && (v0>v1) || (p0<p1) && (v0<v1))) {
          // vref = vref + 100;
          pwm_out = pwm_out - 0.03;
        } else if ((p0<p1) && (v0>v1) || (p0>p1) && (v0<v1)) {
          // vref = vref - 100;
          pwm_out = pwm_out + 0.03;
        } else {
          Serial.println("No increment");
        }

        if (V_1 < V_UPLIM && V_2 < V_UPLIM && V_3 < V_UPLIM) {
            next_state = CHARGE;
            digitalWrite(PIN_YELLED,true);
            // Rationale: Discharge current in the more higher charged cells
            //Connect to discharging relay if a battery is significantly lower  
            if ((SoC_2 - SoC_1) > 5  && (SoC_3 - SoC_1) > 5) {  // Cell 1 Lowest
                disc1 = 0, disc2 = 1, disc3 = 1;
                dq1 = dq1 + current_measure/1000.0;
                dq2 = dq2 + (current_measure - V_2/150.0)/1000.0;
                dq3 = dq3 + (current_measure - V_3/150.0)/1000.0;
                Serial.println("Cell 1 Lowest");
            } else if ((SoC_1 - SoC_2) > 5 && (SoC_3 - SoC_2) > 5) { // Cell 2 Lowest
                disc1 = 1, disc2 = 0, disc3 = 1;
                dq1 = dq1 + (current_measure - V_1/150.0)/1000.0;
                dq2 = dq2 + current_measure/1000.0;
                dq3 = dq3 + (current_measure - V_3/150.0)/1000.0;
                Serial.println("Cell 2 Lowest");
            } else if ((SoC_1 - SoC_3) > 5 && (SoC_2 - SoC_3) > 5)  { // Cell 3 Lowest
                disc1 = 1, disc2 = 1, disc3 = 0;
                dq1 = dq1 + (current_measure- V_1/150.0)/1000.0;
                dq2 = dq2 + (current_measure - V_2/150.0)/1000.0;
                dq3 = dq3 + current_measure/1000.0;
                Serial.println("Cell 3 Lowest");
            } else {
              disc1 = 0, disc2 = 0, disc3 = 0;
                dq1 = dq1 + current_measure/1000.0;
                dq2 = dq2 + current_measure/1000.0;
                dq3 = dq3 + current_measure/1000.0;
                Serial.println("Not balancing");
            }
            digitalWrite(PIN_DISC1, disc1);
            digitalWrite(PIN_DISC2, disc2);
            digitalWrite(PIN_DISC3, disc3);
        } else { // otherwise go to IDLE
          next_state = 2;
          digitalWrite(PIN_YELLED,false);
        }         

        // Reset values for next round
        p1 = p0;
        v1 = v0;
        
        if(input_switch == 0){
          next_state = 0;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case 2:{ // Charge Rest, green LED is off and no current
        current_ref = 0;
        if(input_switch == 0){ // UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(8,false);
        } else {
          next_state = 2;
          digitalWrite(8,false);
        }
        break;        
      }
      case 5: { // ERROR state RED led and no current
        current_ref = 0;
        next_state = 5; // Always stay here
        digitalWrite(7,true);
        digitalWrite(8,false);
        if(input_switch == 0){ //UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(7,false);
        }
        break;
      }     
    }

    // The current is halted for a while when the relay is on.
    if (relay_on == 1) {
      dq1 = dq1*0.9;
      dq2 = dq2*0.9;
      dq3 = dq3*0.9;
      relay_on = 0;
    }

    // SoC Measurement
    mySMPS.compute_SOC(state_num, V_1, V_2, V_3, dq1, dq2, dq3);
    SoC_1 = mySMPS.get_SOC(1);
    SoC_2 = mySMPS.get_SOC(2);
    SoC_3 = mySMPS.get_SOC(3);
    
    // Now Print all values to serial and SD
    dataString = String(state_num) + "," + String(V_1) + "," + String(V_2) + "," + String(V_3) + "," +String(current_measure) + "," + String(disc1) + "," + String(disc2) + "," + String(disc3) + "," + String(V_PD) + "," + String(pwm_out) + "," + String(p0/1000000);
    Serial.println(dataString);
    
    File dataFile = SD.open("PVCELLS.CSV", FILE_WRITE);
    if (dataFile){ 
      dataFile.println(dataString);
    } else {
      Serial.println("File not open"); 
    }
    dataFile.close();

    dq1 = 0; dq2 = 0; dq3 = 0;
  }

  if (int_count == 1000) { // This runs every 5 seconds
    int_count = 0;
  }
}

// Timer A CMP1 interrupt. Every 1000us the program enters this interrupt. This is the fast 1kHz loop
ISR(TCA0_CMP1_vect) {
  loop_trigger = 1; //trigger the loop when we are back in normal flow
  TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_CMP1_bm; //clear interrupt flag
}

float saturation( float sat_input, float uplim, float lowlim) { // Saturation function
  if (sat_input > uplim) sat_input = uplim;
  else if (sat_input < lowlim ) sat_input = lowlim;
  else;
  return sat_input;
}

float pidi(float pid_input) { // discrete PID function
  float e_integration;
  e0i = pid_input;
  e_integration = e0i;

  //anti-windup
  if (u1i >= ui_max) {
    e_integration = 0;
  } else if (u1i <= ui_min) {
    e_integration = 0;
  }

  delta_ui = kpi * (e0i - e1i) + kii * Ts * e_integration + kdi / Ts * (e0i - 2 * e1i + e2i); //incremental PID programming avoids integrations.
  u0i = u1i + delta_ui;  //this time's control output

  //output limitation
  saturation(u0i, ui_max, ui_min);

  u1i = u0i; //update last time's control output
  e2i = e1i; //update last last time's error
  e1i = e0i; // update last time's error
  return u0i;
}

// Voltage PID Controller
float pidv( float pid_input){
  float e_integration;
  e0v = pid_input;
  e_integration = e0v;
 
  //anti-windup, if last-time pid output reaches the limitation, this time there won't be any intergrations.
  if(u1v >= uv_max) {
    e_integration = 0;
  } else if (u1v <= uv_min) {
    e_integration = 0;
  }

  delta_uv = kpv*(e0v-e1v) + kiv*Ts*e_integration + kdv/Ts*(e0v-2*e1v+e2v); //incremental PID programming avoids integrations.there is another PID program called positional PID.
  u0v = u1v + delta_uv;  //this time's control output

  //output limitation
  saturation(u0v,uv_max,uv_min);
  
  u1v = u0v; //update last time's control output
  e2v = e1v; //update last last time's error
  e1v = e0v; // update last time's error
  return u0v;
}
