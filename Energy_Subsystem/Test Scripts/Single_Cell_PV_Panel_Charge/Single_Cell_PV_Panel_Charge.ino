// Charging a single cell with a PV panel
  // No Discharging
// Port A: PV Panel, voltage available at A0
// Port B: A single battery
// Aim: To maximise solar panel output while ensuring that the current is delievered at a safe level.
  // Instead of using a reference voltage, alter PWM instead
  // Asynchronous Buck

#include <Wire.h>
#include <INA219_WE.h>
#include <SPI.h>
#include <SD.h>
#include <MovingAverage.h>
#include "Power.h"

SMPS mySMPS;
INA219_WE ina219; // this is the instantiation of the library for the current sensor

#define PIN_OLCL 2
#define PIN_VA A1
#define PIN_VB A0
#define PIN_PWM 6
#define PIN_REDLED 7
#define PIN_YELLED 8

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

const int chipSelect = 10;
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

float V_Bat; float V_PD;
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
float p_diff, v_diff, i_diff;
float dq1;

int arr_size = 0;

void setup() {
  //Some General Setup Stuff

  Wire.begin(); // We need this for the i2c comms for the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  ina219.init(); // this initiates the current sensor
  Serial.begin(9600); // USB Communications

  if (SD.exists("MPPT_PVC.csv")) {
      SD.remove("MPPT_PVC.csv");
    }

  mySMPS.init();
 
  noInterrupts(); //disable all interrupts
  analogReference(EXTERNAL); // We are using an external analogue reference for the ADC

  //SMPS Pins
  pinMode(13, OUTPUT); // Using the LED on Pin D13 to indicate status
  pinMode(2, INPUT_PULLUP); // Pin 2 is the input from the CL/OL switch
  pinMode(6, OUTPUT); // This is the PWM Pin

  //LEDs on pin 7 and 8
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);

  //Analogue input, the battery voltage (also port B voltage)
  pinMode(A0, INPUT);

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
      V_Bat = analogRead(A0)*4.096/1.03*3; //manual correction for potential divider
      V_PD = analogRead(A1)*4.096/1.03* 4.1626; //mannual correction for potential divider
      if ((V_Bat > 3700 || V_Bat < 2400)) { //Checking for Error states (just battery voltage for now)
          state_num = 5; //go directly to jail
          next_state = 5; // stay in jail
          digitalWrite(7,true); //turn on the red LED
          current_ref = 0; // no current
      }
      current_measure = (ina219.getCurrent_mA()); // sample the inductor current (via the sensor chip)
      
      //TODO: determine current in
      I_in = V_Bat * current_measure / V_PD * 0.5; // assuming 50% efficiency
      
//      ev = (vref - V_PD)/1000;  //voltage error at this time
//      cv = pidv(ev);  //voltage pid
//      cv = saturation(cv, current_limit, 0); //current demand saturation
//      ei = (cv - I_in)/1000; //current error
//      closed_pwm = pidi(ei);  //current pid
      pwm_out = saturation(pwm_out, 0.99, 0.01);  //duty_cycle saturation
      analogWrite(6, (int)(255 - pwm_out * 255)); // write it out (inverting for the Buck here)
      int_count++; //count how many interrupts since this was last reset to zero
      loop_trigger = 0; //reset the trigger and move on with life
  }
  
  if (int_count == 200) { // SLOW LOOP (1Hz)
    input_switch = digitalRead(2); //get the OL/CL switch status
    switch (state_num) { // STATE MACHINE (see diagram)
      case 0:{ // Start state (no current, no LEDs)
        current_ref = 0;
        pwm_out = 0;
        if (input_switch == 1) { // if switch, move to charge
          // First time, so reset voltage panel values
          pwm_out = 0.5; // initial pwm value   
          v1 = V_PD;
          i1 = current_measure;
          p1 = v1*i1;
          pwm_out = pwm_out + 0.03;

          next_state = 1;
          digitalWrite(8,true);
        } else { // otherwise stay put
          next_state = 0;
          digitalWrite(8,false);
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
        i_diff = i0-i1;

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
        

        // Incremental Conductance Algorithm
        /*
        if (((abs(v_diff) >= 10) && (i_diff/v_diff > -i0/v0) || (abs(v_diff) < 10) && (i_diff > 0))) {
          // vref = vref + 100;
          pwm_out = pwm_out - 0.05;
        } else if ((abs(v_diff) >= 10) && (i_diff/v_diff <= -i0/v0) || (abs(v_diff) < 10) && (i_diff <= 0)) {
          // vref = vref - 100;
          pwm_out = pwm_out + 0.05;   
        } else {
          Serial.println("Not incrementing or decrementing");
        }
        */
        
        if (V_Bat < 3600) { // if not charged, stay put
          next_state = 1;
          dq1 = dq1 + current_measure/1000;
          digitalWrite(8,true);          
        } else { // otherwise go to charge rest
          next_state = 2;
          digitalWrite(8,false);
        }

        // Reset values for next round
        p1 = p0;
        v1 = v0;
        
        if(input_switch == 0){ // UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(8,false);
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
      default :{ // Should not end up here ....
        Serial.println("Boop");
        current_ref = 0;
        next_state = 5; // So if we are here, we go to error
        digitalWrite(7,true);
      }
      
    }

    // SoC Measurement
    mySMPS.compute_SOC(state_num, V_Bat, 0, 0, dq1, 0, 0);
    
    dataString = String(state_num) + "," + String(pwm_out) + "," + String(V_Bat) + "," + String(current_ref) + "," + String(current_measure) + "," + String(V_PD) + "," + String(V_PD*current_measure/1000000); //build a datastring for the CSV file
    Serial.println(dataString); // send it to serial as well in case a computer is connected
    File dataFile = SD.open("MPPT_PVC.csv", FILE_WRITE); // open our CSV file
    if (dataFile){ //If we succeeded (usually this fails if the SD card is out)
      dataFile.println(dataString); // print the data
    } else {
      Serial.println("File not open"); //otherwise print an error
    }
    dataFile.close(); // close the file
    
    int_count = 0; // reset the interrupt count so we dont come back here for 1000ms
    dq1 = 0;
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
