/* 

initial state of charge for cells (coulombs)
Cell 1: 1793
Cell 2: 2000.5
Cell 3: 1921.75

FOR TESTING BALANCING ONLY: Buck Discharging
  PORT B: 5V Wall Plug, 75R resistors
  PORT A: 3 batteries

Extrapolate output current
Assuming 100% efficiency (to be safe)

Aim:
This is a simplified variant of the master file.
Obtain to allow charging and charging at designated speeds
  Charging at 250mA (or 500mA short periods) from wall
  Discharging at 500mA (or 1A short periods)
Evaluates the SoC using the SoC voltage lookup table

3 CELLS IN SERIES!!!!

Flow chart
typical: 0 > 1 > 6 > 2 > 8 > 4 > 1 > 6 > 2 > 8 > 4 > ......

0 IDLE
1 CHARGE (yellow LED)
2 CHARGE REST (note: only records charge data after first discharge)
3 SLOW DISCHARGE (250mA)
4 DISCHARGE REST
5 ERROR (red LED)(must go to 0 next and restart)
6 CONSTANT VOLTAGE CHARGE (blinking yellow LED)
7 RECALIBRATION COMPLETE
8 NORMAL DISCHARGE (500mA)
9 RAPID DISCHARGE (1A)
10 RAPID CURRENT CHARGE (500mA)
  Normal discharge (8), rapid discharge (1A) and rapid current charge (500mA) are currently called manually
  In general rapid discharge and rapid current charge is not recommended for long periods.
    Rapid discharge is disabled when SoC is below 30%. Also it is only valid for no more than 10 seconds.
    Rapid charge is disabled when SoC is above 70%

Rationale for SOC
    SOC is only evaluated every 60 seconds, and not within the file.
    If voltage is within a certain voltage threshold, then coulomb counting is used
    If voltage is above or beyond a certain threshold, then open circuit voltage is used, alongside a moving average.
*/

#include <Wire.h>
#include <INA219_WE.h>
#include <SPI.h>
#include <SD.h>
#include <MovingAverage.h>
#include "Power.h"

INA219_WE ina219; // this is the instantiation of the library for the current sensor

#define PIN_OLCL 2
#define PIN_PWM 6
#define PIN_REDLED 7
#define PIN_YELLED 8
#define PIN_V1 A1
#define PIN_V2 A2
#define PIN_V3 A3
#define PIN_DISC1 A7
#define PIN_DISC2 A6
#define PIN_DISC3 A0
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

// Setup SMPS
SMPS mySMPS;

// set up variables using the SD utility library functions:
const int chipSelect = 10; const int SD_CS = 10;
Sd2Card card;
SdVolume volume;
SdFile root;
String dataString;

unsigned int rly_timer = 0;
unsigned int rest_timer;
unsigned int rapid_timer;
unsigned int loop_trigger;
unsigned int int_count = 0; // a variables to count the interrupts. Used for program debugging.

// Voltage PID Controller
float ev=0,cv=0,ei=0; //internal signals // FIXME:
  // ev: difference between V_ref and V_b
  // cv: current obtained from voltage PID controller. need to saturate it
  // ei: difference between desired and inductor current. error_amps in this case? FIXME:
float kpv=0.05024,kiv=15.78,kdv=0; // voltage pid.
float u0v,u1v,delta_uv,e0v,e1v,e2v; // u->output; e->error; 0->this time; 1->last time; 2->last last time
float uv_max=4, uv_min=0; //anti-windup limitation
float vref=0;

// Current PID Controller
float u0i, u1i, delta_ui, e0i, e1i, e2i; // Internal values for the current controller
float ui_max = 1, ui_min = 0; //anti-windup limitation
float kpi = 0.02512, kii = 39.4, kdi = 0; // current pid.
float Ts = 0.001; //1 kHz control frequency.
float current_measure, current_ref = 0, error_amps; // Current Control
float pwm_out;

// Series Batteries Variables
float V_1 = 0, V_2 = 0, V_3 =0;
float SoC_1 = 0, SoC_2 = 0, SoC_3 = 0;
float V_UPLIM = 3590;
float V_LOWLIM = 2500;

// State Machine Stuff
boolean input_switch;
int state_num = 0,next_state, prev_state = -1;
bool recalibrating = 0; bool discharged = 0;
bool stop = 0;

// Blinking LED for state 6
boolean blink = 0;

// Current Capacity: Only calculated during discharge process
float q1 = 0, q2 = 0, q3 = 0;

// Stores the amount of charge added/removed within the past 2 minutes. Reset after.
float dq1 = 0, dq2 = 0, dq3 = 0;

// Account for difference in current when relay is on;
bool disc1 = 0, disc2 = 0, disc3 = 0;
bool relay_on = 0;

void setup() {

  //Some General Setup Stuff
  Wire.begin(); // We need this for the i2c comms for the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  ina219.init(); // this initiates the current sensor
  Serial.begin(9600); // USB Communications

  mySMPS.init();

  if (SD.exists("BatCycle.csv")) { // Wipe the datalog when starting
    SD.remove("BatCycle.csv");
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

  // TimerA0 initialization for (NOW) 0.2kHz control-loop interrupt.
  TCA0.SINGLE.PER = 4999; //
  TCA0.SINGLE.CMP1 = 4999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm;

  // TimerB0 initialization for PWM output
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz

  interrupts();  //enable interrupts.
  analogWrite(PIN_PWM, 120); //just a default state to start with
  
}

void loop() {

//In rests state, if command was running (but not calibration), command complete
  if (loop_trigger == 1){ // FAST LOOP (0.2kHZ)
      state_num = next_state; //state transition
      
      //Checking for Error states (individual battery voltages defined)
      if ((V_1 > 3700 || V_1 < 2400) || (V_2 > 3700 || V_2 < 2400) || (V_3 > 3700 || V_3 < 2400)) { 
          state_num = ERROR; //go directly to jail
          next_state = ERROR; // stay in jail
          digitalWrite(PIN_REDLED,true); //turn on the red LED
          current_ref = 0; // no current
      }
      current_measure = (ina219.getCurrent_mA()); // sample the inductor current (via the sensor chip)
     
      // Use constant voltage with respect to battery 1
      // Use voltage then current PID controller for constant voltage (only in state 6)
      if (vref == 3600) { 
        ev = (vref - V_1)/1000;  //voltage error at this time
        pwm_out = pidv(ev);  //voltage pid
        // cv = saturation(cv, 0.25, 0); //current demand saturation
        // ei = (cv - current_measure)/1000; ; //current error
        // pwm_out = pidi(ei);  //current pid
      } else if (vref == 0) { // Use Current PID controller in all other scenarios
        error_amps = (current_ref - current_measure) / 1000; //PID error calculation
        pwm_out = pidi(error_amps); //Perform the PID controller calculation       
      }
      pwm_out = saturation(pwm_out, 0.99, 0.01); //duty_cycle saturation. NOT FIXME: PWM Modulate
      analogWrite(PIN_PWM, (int)(255 - pwm_out * 255)); // write it out (inverting for the Buck here)
      int_count++; //count how many interrupts since this was last reset to zero
      rly_timer++;
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
     // Account for halted current during relay. 26 time units in total, or 26/200s (1-26/200) = 0.87
     relay_on = 1;
  }
 
  // This still runs every 1 second
  if (int_count % 200 == 0) { // SLOW LOOP (0.2Hz)
    input_switch = digitalRead(PIN_OLCL); //get the OL/CL switch status
    switch (state_num) { // STATE MACHINE (see diagram)
      case IDLE:{ // 0 Idle state (no current, no LEDs)
        current_ref = 0;
        q1 = 0; q2 = 0; q3 = 0;
        // dq1 = dq1; dq2 = dq2; dq3 = dq3; // dq value is frozen
        if (input_switch == 1) { // if switch, move to charge
          next_state = SLOW_DISCHARGE;
          digitalWrite(PIN_YELLED,true);
        } else { // otherwise stay put
          next_state = IDLE;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case DISCHARGE_REST:{ // 4 Discharge rest, no LEDs no current
        current_ref = 0;
        vref = 0;
        // dq1 = dq1; dq2 = dq2; dq3 = dq3; // dq value is frozen
        if(input_switch == 0){
          next_state = IDLE;
          rest_timer = 0;
          digitalWrite(PIN_YELLED,false);
        }
        digitalWrite(PIN_YELLED,true);
        digitalWrite(PIN_REDLED,true);
        next_state = DISCHARGE_REST;
        break;
      }
      case ERROR: { // 5 ERROR state RED led and no current
        current_ref = 0;
        // dq1 = dq1; dq2 = dq2; dq3 = dq3; // dq value is frozen
        if(input_switch == 0){
          next_state = IDLE;
          digitalWrite(PIN_REDLED,false);
          digitalWrite(PIN_YELLED,false);
        } else {
          next_state = ERROR;
          digitalWrite(PIN_REDLED,true);
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case SLOW_DISCHARGE: { // 3 Slow discharge (-500mA)
        current_ref = -250;
        if (V_1 > V_LOWLIM && V_2 > V_LOWLIM && V_3 > V_LOWLIM) { // While not at minimum volts, stay here
          next_state = SLOW_DISCHARGE;
          digitalWrite(PIN_YELLED,false);
          
          if ((SoC_2 - SoC_1) > 3  && (SoC_3 - SoC_1) > 3) {  // Cell 1 Lowest
                Serial.println("Cell 1 lowest");
                disc1 = 1, disc2 = 0, disc3 = 0;
                dq1 = dq1 + (current_measure + V_1/150/1000)/1000;
                dq2 = dq2 + current_measure/1000;
                dq3 = dq3 + current_measure/1000;
            } else if ((SoC_1 - SoC_2) > 3 && (SoC_3 - SoC_2) > 3) { // Cell 2 Lowest
                Serial.println("Cell 2 lowest");
                disc1 = 0, disc2 = 1, disc3 = 0;
                dq1 = dq1 + current_measure/1000;
                dq2 = dq2 + (current_measure + V_2/150/1000)/1000;
                dq3 = dq3 + current_measure/1000;
            } else if ((SoC_1 - SoC_3) > 3 && (SoC_2 - SoC_3) > 3) { // Cell 3 Lowest
                Serial.println("Cell 3 lowest");
                disc1 = 0, disc2 = 0, disc3 = 1;
                dq1 = dq1 + current_measure/1000;
                dq2 = dq2 + current_measure/1000;
                dq3 = dq3 + (current_measure + V_3/150/1000)/1000;
            } else {
              Serial.println("No balancing");
                disc1 = 0, disc2 = 0, disc3 = 0;
                dq1 = dq1 + current_measure/1000;
                dq2 = dq2 + current_measure/1000;
                dq3 = dq3 + current_measure/1000;
            }
            digitalWrite(PIN_DISC1, disc1);
            digitalWrite(PIN_DISC2, disc2);
            digitalWrite(PIN_DISC3, disc3);

        } else { // If we reach full discharged, move to rest
          next_state = DISCHARGE_REST;
          digitalWrite(PIN_YELLED,false);
        }
        if(input_switch == 0){
          next_state = IDLE;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }  
    }

    // The current is halted for a while when the relay is on.
    if (relay_on == 1) {
      dq1 = dq1*0.87;
      dq2 = dq2*0.87;
      dq3 = dq3*0.87;
      relay_on = 0;
    }
    
    // SoC Measurement
    mySMPS.compute_SOC(state_num, V_1, V_2, V_3, dq1, dq2, dq3);
    SoC_1 = mySMPS.get_SOC(1);
    SoC_2 = mySMPS.get_SOC(2);
    SoC_3 = mySMPS.get_SOC(3);
  
    // Now Print all values to serial and SD
    dataString = String(state_num) + "," + String(V_1) + "," + String(V_2) + "," + String(V_3) + "," + String(current_ref) + "," +String(current_measure) + "," + String(disc1) + "," + String(disc2) + "," + String(disc3);
    Serial.println(dataString);
    
    File dataFile = SD.open("BatCycle.csv", FILE_WRITE);
    if (dataFile){ 
      dataFile.println(dataString);
    } else {
      Serial.println("File not open"); 
    }
    dataFile.close();

    rly_timer = 0;
    dq1 = 0; dq2 = 0; dq3 = 0;
  }

  if (int_count == 1000) { // read voltage every 5 seconds
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

// Current PID function
float pidi(float pid_input) { 
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
float pidv(float pid_input){
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
