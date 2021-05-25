/* 

initial state of charge for cells (coulombs)
Cell 1: 1793
Cell 2: 2000.5
Cell 3: 1921.75

Aim:
Obtain to allow charging and charging at designated speeds
  Charging at 250mA (or 500mA short periods)
  Discharging at 500mA (or 1A short periods)
Evaluates the SoC using the SoC voltage lookup table

3 CELLS IN SERIES!!!!

Flow chart
typical: 0 > 1 > 2 > 3 > 4 > 1 > 2 > 3 > 4 > ......
recalibrate: 0 > 1 > 6 > 2 > 3 > 4 > 1 > 6 > 2 > 7 > 0
discharge: 0 > 8 > 4 > 0
rapid_discharge: 0 > 9 > 8 > 4 > 0
charge: 0 > 1 > 6 > 2 > 0
rapid_charge: 0 > 10 > 1 > 6 > 2 > 0

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
#include <Power.h>

INA219_WE ina219; // this is the instantiation of the library for the current sensor
SMPS mySMPS;

// Which cell are we using?
int CELL = 1;

#define PIN_OLCL 2
#define PIN_PWM 6
#define PIN_REDLED 7
#define PIN_YELLED 8
#define PIN_V1 A1
#define PIN_V2 A2
#define PIN_V3 A3
#define PIN_DISC1 5
#define PIN_DISC2 4
#define PIN_DISC3 9
#define PIN_RLY1 A7
#define PIN_RLY2 A6
#define PIN_RLY3 A0

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

// set up variables using the SD utility library functions:
const int chipSelect = 10;
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
float V_UPLIM = 3590;
float V_UPBALLIM = 3300;
float V_LOWBALLIM = 3100;
float V_LOWLIM = 2500;

// State Machine Stuff
boolean input_switch;
int state_num = 0,next_state;
bool recalibrating = 0; bool discharged = 0;
bool stop = 0;

// Blinking LED for state 6
boolean blink = 0;

// Current Capacity: Only calculated during discharge process
float q1 = 0, q2 = 0, q3 = 0;

// Stores the amount of charge added/removed within the past 2 minutes. Reset after.
float dq1 = 0, dq2 = 0, dq3 = 0; 

void setup() {

  //Some General Setup Stuff
  Wire.begin(); // We need this for the i2c comms for the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  ina219.init(); // this initiates the current sensor
  Serial.begin(9600); // USB Communications

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

  // TimerA0 initialization for 1kHz control-loop interrupt.
  TCA0.SINGLE.PER = 999; //
  TCA0.SINGLE.CMP1 = 999; //

void loop() {

    //TODO: command reception loop
    if (Serial.available() && mySMPS.command_running == 0 ) {
        mySMPS.command_running = 1;
        char message[30];
        int amount = Serial.readBytesUntil(';', message, 30);
        message[amount] = NULL;

        int data[10];
        int count = 0;
        char* offset = message;
        while (true) {
            data[count++] = atoi(offset);
            offset = strchr(offset, ',');
            if (offset) offset++;
            else break;
        }

        // mDistance = data[0];
        // mSpeed = data[1];
        // mDirection = data[2];

        mySMPS.decodeCommand(); //TODO: Implement decoding
        next_state = mySMPS.get_state();
        recalibrating = mySMPS.get_recalibrate();
    }

//In rests state, if command was running (but not calibration), command complete
  if (loop_trigger == 1){ // FAST LOOP (1kHZ)
      state_num = next_state; //state transition
       
      //check the battery voltage (1.03 is a correction for measurement error, you need to check this works for you)
      V_Bat = analogRead(A0)*4.096/1.03;

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
  if (rly_timer == 400) { // Relay switching is 10ms. Double for safety
    analogWrite(PIN_RLY1,1);
  } else if (rly_timer == 420) { // Read battery 1 voltage
    V_1 = analogRead(PIN_V1)*4.096/1.03;
  } else if (rly_timer == 430) {
    analogWrite(PIN_RLY1,0);
  } else if (rly_timer == 450) {
    analogWrite(PIN_RLY2,1);
  } else if (rly_timer == 470) {
    V_2 = analogRead(PIN_V2)*4.096/1.03;
  } else if (rly_timer == 480) {
    analogWrite(PIN_RLY2,0);
  } else if (rly_timer == 500) {
    analogWrite(PIN_RLY3,1);
  } else if (rly_timer == 520) {
    V_3 = analogRead(PIN_V3)*4.096/1.03;
  } else if (rly_timer == 530) {
    analogWrite(PIN_RLY3,0);
  }
 
  // This still runs every second
  if (int_count % 1000 == 0) { // SLOW LOOP (1Hz)
    input_switch = digitalRead(PIN_OLCL); //get the OL/CL switch status
    switch (state_num) { // STATE MACHINE (see diagram)
      case IDLE:{ // 0 Idle state (no current, no LEDs)
        current_ref = 0;
        q1 = 0; q2 = 0; q3 = 0;
        // dq1 = dq1; dq2 = dq2; dq3 = dq3; // dq value is frozen
        if (input_switch == 1) { // if switch, move to charge
          next_state = CHARGE;
          digitalWrite(PIN_YELLED,true);
        } else { // otherwise stay put
          next_state = IDLE;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case CHARGE:{ // 1 Charge state (250mA and a green LED)
        current_ref = 250;
        if (V_1 < V_UPLIM && V_2 < V_UPLIM && V_3 < V_UPLIM) {
            next_state = CHARGE;
            digitalWrite(PIN_YELLED,true);
            // Start balancing when above V_UPBALLIM (3300mV)
            // Rationale: Discharge current in the more higher charged cells
            if (V1 > V_UPBALLIM && V_2 > V_UPBALLIM && V_3 > V_UPBALLIM) {
                //Connect to discharging relay if a battery is significantly lower  
                if (V_2 - V_1 > 100  && V_3 - V_1 > 100) {  // Cell 1 Lowest
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, true);
                    digitalWrite(PIN_DISC3, true);
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + (current_measure - V_2/150)/1000;
                    dq3 = dq3 + (current_measure - V_3/150)/1000;
                } else if (V_1 - V_2 > 100 && V_3 - V_2 > 100) { // Cell 2 Lowest
                    digitalWrite(PIN_DISC1, true);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, true);
                    dq1 = dq1 + (current_measure - V_1/150)/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + (current_measure - V_3/150)/1000;
                } else if (V_1 - V_3 > 100 && V_2 - V_3 > 100) { // Cell 3 Lowest
                    digitalWrite(PIN_DISC1, true);
                    digitalWrite(PIN_DISC2, true);
                    digitalWrite(PIN_DISC3, false);
                    dq1 = dq1 + (current_measure - V_1/150)/1000;
                    dq2 = dq2 + (current_measure - V_2/150)/1000;
                    dq3 = dq3 + current_measure/1000;
                } else {
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, false);
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + current_measure/1000;
                }
            } else {
                digitalWrite(PIN_DISC1, false);
                digitalWrite(PIN_DISC2, false);
                digitalWrite(PIN_DISC3, false);
                dq1 = dq1 + current_measure/1000;
                dq2 = dq2 + current_measure/1000;
                dq3 = dq3 + current_measure/1000;
            }
        } else { // otherwise go to constant voltage charge
          next_state = CV_CHARGE;
          digitalWrite(PIN_YELLED,false);
        }      
        if(mySMPS.command_running == 0){
          next_state = IDLE;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case CHARGE_REST:{ // 2 Charge Rest, green LED is off and no current
        current_ref = 0;
        // dq1 = dq1; dq2 = dq2; dq3 = dq3; // dq value is frozen
        if(mySMPS.command_running == 1 && recalibrating == 0){
          next_state = IDLE;
          rest_timer = 0;
          digitalWrite(PIN_YELLED,false);
          mySMPS.command_running = 0;
        }
        if (recalibrating == 1) {
            if (rest_timer < 30) { // Stay here if timer < 30
                next_state = CHARGE_REST;
                digitalWrite(PIN_YELLED,false);
                rest_timer++;
            } else { // Move to completion state if battery has already been discharged
                if (discharged == 1) {
                    next_state = RECAL_DONE;
                    digitalWrite(PIN_YELLED,false);
                    rest_timer = 0;
                } else { // Otherwise discharge
                    next_state = SLOW_DISCHARGE;
                    digitalWrite(PIN_YELLED,false);
                    rest_timer = 0;
                }    
            }
        }
        break;        
      }
      case SLOW_DISCHARGE:{ //3 Slow Discharge state (-250mA and no LEDs)
         current_ref = -250;
         // Rationale: reduce discharge current from the cell with the lowest voltage
         if (V_1 > V_LOWLIM && V_2 > V_LOWLIM && V_3 > V_LOWLIM) { // While not at minimum volts, stay here
            next_state = SLOW_DISCHARGE;
            digitalWrite(PIN_YELLED,false);
            if (V_1 < V_LOWBALLIM && V_2 < V_LOWBALLIM && V_3 < V_LOWBALLIM) {
                if (V_2 - V_1 > 100  && V_3 - V_1 > 100) {  // Cell 1 Lowest
                    digitalWrite(PIN_DISC1, true);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, false);
                    q1 = q1 + (current_measure - V_1/150)/1000;
                    q2 = q2 + current_measure/1000;
                    q3 = q3 + current_measure/1000;
                    dq1 = dq1 + (current_measure - V_1/150)/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + current_measure/1000;
                } else if (V_1 - V_2 > 100 && V_3 - V_2 > 100) { // Cell 2 Lowest
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, true);
                    digitalWrite(PIN_DISC3, false);
                    q1 = q1 + current_measure/1000;
                    q2 = q2 + (current_measure - V_2/150)/1000;
                    q3 = q3 + current_measure/1000;
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + (current_measure - V_2/150)/1000;
                    dq3 = dq3 + current_measure/1000;
                } else if (V_1 - V_3 > 100 && V_2 - V_3 > 100) { // Cell 3 Lowest
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, true);
                    q1 = q1 + current_measure/1000;
                    q2 = q2 + current_measure/1000;
                    q3 = q3 + (current_measure - V_3/150)/1000;
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + (current_measure - V_3/150)/1000;
                } else {
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, false);
                    q1 = q1 + current_measure/1000;
                    q2 = q2 + current_measure/1000;
                    q3 = q3 + current_measure/1000;
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + current_measure/1000;
                } 
           } else {
                digitalWrite(PIN_DISC1, false);
                digitalWrite(PIN_DISC2, false);
                digitalWrite(PIN_DISC3, false);
                q1 = q1 + current_measure/1000;
                q2 = q2 + current_measure/1000;
                q3 = q3 + current_measure/1000;
           }            
         } else { // If we reach full discharged, move to rest
           next_state = DISCHARGE_REST;
           digitalWrite(PIN_YELLED,false);
         }
        if(mySMPS.command_running == 0){
          next_state = IDLE;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case DISCHARGE_REST:{ // 4 Discharge rest, no LEDs no current
        current_ref = 0;
        // dq1 = dq1; dq2 = dq2; dq3 = dq3; // dq value is frozen
        if(mySMPS.command_running == 1 && recalibrating == 0){
          next_state = IDLE;
          rest_timer = 0;
          digitalWrite(PIN_YELLED,false);
          mySMPS.command_running = 0;
        }
        if (recalibrating == 1) {
            discharged = 1;
            if (rest_timer < 30) { // Rest here for 30s like before
                next_state = DISCHARGE_REST;
                digitalWrite(PIN_YELLED,false);
                rest_timer++;
            } else { // When thats done, move back to charging (and light the green LED)
                mySMPS.send_current_cap(q1, q2, q3); // coulomb counting during discharge
                next_state = CHARGE;
                digitalWrite(PIN_YELLED,true);
                rest_timer = 0;
            }
        }
        break;
      }
      case ERROR: { // 5 ERROR state RED led and no current
        current_ref = 0;
        // dq1 = dq1; dq2 = dq2; dq3 = dq3; // dq value is frozen   
        if(mySMPS.command_running == 1){
            mySMPS.triggerError();
            next_state = ERROR; // Always stay here
            digitalWrite(PIN_REDLED,true);
            digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case CV_CHARGE: { // 6 Charging with constant voltage (after state 1, before 2)
        vref = 3600;
        current_ref = 0;
        q1 = q1 + current_measure/1000;
        q2 = q2 + current_measure/1000;
        q3 = q3 + current_measure/1000;
        if (current_measure < 0) {
            next_state = CHARGE_REST;
            vref=0;
        }
        if (blink == 1) { // Blink LED in state 6
            digitalWrite(PIN_YELLED,true);
            blink = 0;
        } else {
            digitalWrite(PIN_YELLED,false);
            blink = 1;
        }
        if(mySMPS.command_running == 0){
            next_state = IDLE;
            digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case RECAL_DONE: { // 7 Recalibration Complete
          recalibrating = 0;
          discharged = 0;
          //TODO: evaluate SOC stats
          break;
      }
      case DISCHARGE: { // 8 Normal discharge (-500mA)
        current_ref = -500;
         if (V1 > V_LOWLIM && V_2 > V_LOWLIM && V_3 > V_LOWLIM) { // While not at minimum volts, stay here
            next_state = DISCHARGE;
            digitalWrite(PIN_YELLED,false);
            if (V_1 < V_LOWBALLIM && V_2 < V_LOWBALLIM && V_3 < V_LOWBALLIM) {
                if (V_2 - V_1 > 100  && V_3 - V_1 > 100) {  // Cell 1 Lowest
                    digitalWrite(PIN_DISC1, true);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, false);
                    dq1 = dq1 + (current_measure - V_1/150)/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + current_measure/1000;
                } else if (V_1 - V_2 > 100 && V_3 - V_2 > 100) { // Cell 2 Lowest
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, true);
                    digitalWrite(PIN_DISC3, false);
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + (current_measure - V_2/150)/1000;
                    dq3 = dq3 + current_measure/1000;
                } else if (V_1 - V_3 > 100 && V_2 - V_3 > 100) { // Cell 3 Lowest
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, true);
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + (current_measure - V_3/150)/1000;
                } else {
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, false);
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + current_measure/1000;
                } 
            } else {
                digitalWrite(PIN_DISC1, false);
                digitalWrite(PIN_DISC2, false);
                digitalWrite(PIN_DISC3, false);
                q1 = q1 + current_measure/1000;
                q2 = q2 + current_measure/1000;
                q3 = q3 + current_measure/1000;
                dq1 = dq1 + current_measure/1000;
                dq2 = dq2 + current_measure/1000;
                dq3 = dq3 + current_measure/1000;
            }
         } else { // If we reach full discharged, move to rest
           next_state = DISCHARGE_REST;
           digitalWrite(PIN_YELLED,false);
         }
        if(mySMPS.command_running == 0){
          next_state = IDLE;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case RAPID_DISCHARGE: { // Rapid discharge (-1A)
        current_ref = -1000;
         if ((V1 > V_LOWLIM && V_2 > V_LOWLIM && V_3 > V_LOWLIM) && rapid_timer < 10) { // while timer is less than 10 seconds
           next_state = RAPID_DISCHARGE;
           digitalWrite(PIN_YELLED,false);
            if (V_1 < V_LOWBALLIM && V_2 < V_LOWBALLIM && V_3 < V_LOWBALLIM) {
                if (V_2 - V_1 > 100  && V_3 - V_1 > 100) {  // Cell 1 Lowest
                    digitalWrite(PIN_DISC1, true);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, false);
                    dq1 = dq1 + (current_measure - V_1/150)/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + current_measure/1000;
                } else if (V_1 - V_2 > 100 && V_3 - V_2 > 100) { // Cell 2 Lowest
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, true);
                    digitalWrite(PIN_DISC3, false);
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + (current_measure - V_2/150)/1000;
                    dq3 = dq3 + current_measure/1000;
                } else if (V_1 - V_3 > 100 && V_2 - V_3 > 100) { // Cell 3 Lowest
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, true);
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + (current_measure - V_3/150)/1000;
                } else {
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, false);
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + current_measure/1000;
                } 
            } else {
                digitalWrite(PIN_DISC1, false);
                digitalWrite(PIN_DISC2, false);
                digitalWrite(PIN_DISC3, false);
                dq1 = dq1 + current_measure/1000;
                dq2 = dq2 + current_measure/1000;
                dq3 = dq3 + current_measure/1000;
            }
           rapid_timer++;
         } else { // If we reach full discharged, move to rest
           rapid_timer = 0;
           next_state = DISCHARGE;
           digitalWrite(PIN_YELLED,false);
         }
        if(mySMPS.command_running == 0){
          next_state = IDLE;
          digitalWrite(PIN_YELLED,false);
        }      
        break;
      }
      case RAPID_CHARGE: { // Rapid charge (+500mA, with green LED on)
        current_ref = 500;
        if (rapid_timer < 10) {
            if (V_1 < V_UPLIM && V_2 < V_UPLIM && V_3 < V_UPLIM) {
                next_state = RAPID_CHARGE;
                digitalWrite(PIN_YELLED,true);                              
                // Start balancing when above V_UPBALLIM (3300mV)
                // Rationale: Discharge current in the more higher charged cells
                if (V1 > V_UPBALLIM && V_2 > V_UPBALLIM && V_3 > V_UPBALLIM) {
                    //Connect to discharging relay if a battery is significantly lower  
                    if (V_2 - V_1 > 100  && V_3 - V_1 > 100) {  // Cell 1 Lowest
                        digitalWrite(PIN_DISC1, false);
                        digitalWrite(PIN_DISC2, true);
                        digitalWrite(PIN_DISC3, true);
                        dq1 = dq1 + current_measure/1000;
                        dq2 = dq2 + (current_measure - V_2/150)/1000;
                        dq3 = dq3 + (current_measure - V_3/150)/1000;
                    } else if (V_1 - V_2 > 100 && V_3 - V_2 > 100) { // Cell 2 Lowest
                        digitalWrite(PIN_DISC1, true);
                        digitalWrite(PIN_DISC2, false);
                        digitalWrite(PIN_DISC3, true);
                        dq1 = dq1 + (current_measure - V_1/150)/1000;
                        dq2 = dq2 + current_measure/1000;
                        dq3 = dq3 + (current_measure - V_3/150)/1000;
                    } else if (V_1 - V_3 > 100 && V_2 - V_3 > 100) { // Cell 3 Lowest
                        digitalWrite(PIN_DISC1, true);
                        digitalWrite(PIN_DISC2, true);
                        digitalWrite(PIN_DISC3, false);
                        dq1 = dq1 + (current_measure - V_1/150)/1000;
                        dq2 = dq2 + (current_measure - V_2/150)/1000;
                        dq3 = dq3 + current_measure/1000;
                    } else {
                        digitalWrite(PIN_DISC1, false);
                        digitalWrite(PIN_DISC2, false);
                        digitalWrite(PIN_DISC3, false);
                        dq1 = dq1 + current_measure/1000;
                        dq2 = dq2 + current_measure/1000;
                        dq3 = dq3 + current_measure/1000;
                    }
                } else {
                    digitalWrite(PIN_DISC1, false);
                    digitalWrite(PIN_DISC2, false);
                    digitalWrite(PIN_DISC3, false);
                    dq1 = dq1 + current_measure/1000;
                    dq2 = dq2 + current_measure/1000;
                    dq3 = dq3 + current_measure/1000;
                }
            } else { // otherwise go to constant voltage charge
                next_state = CV_CHARGE;
                digitalWrite(PIN_YELLED, false);
            }
        } else {
            next_state = CHARGE;
            digitalWrite(PIN_YELLED, true);
        }
        
        if(mySMPS.command_running == 0){
          next_state = 0;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      default :{ // Should not end up here ....
        Serial.println("Boop");
        current_ref = 0;
        charge_diff = charge_diff;
        next_state = ERROR; // So if we are here, we go to error
        digitalWrite(PIN_REDLED,true);
        break;
      }    
    }
    rly_timer = 0;
  }
  
  // Only deal with SOC every 2 minutes
  if (int_count = 120000) {
      // OCV: Assume that voltage hasn't drastically changed within past 2 minutes
      // Coulomb counting: charge_diff is adding up the charge (current * time) within the 2 mins
      // adjust for difference when discharge circuit is ON
      if (recalibrating) {
          mySMPS.record_curve(state_num, V_1, V_2, V_3);
      } else {
          mySMPS.compute_SOC(state_num, V_1, V_2, V_3, dq1, dq2, dq3);
      }      
      dq1 = 0; dq2 = 0; dq3 = 0;
      int_count = 0; // reset the interrupt count so we dont come back here for 120,000ms (2 minutes)
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