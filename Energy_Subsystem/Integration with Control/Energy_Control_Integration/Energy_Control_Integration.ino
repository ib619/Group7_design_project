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
#include "Power.h"
#include "ControlInterface.h"

INA219_WE ina219; // this is the instantiation of the library for the current sensor

SMPS mySMPS;
ControlInterface ci(&Serial1);

// Which cell are we using?
int CELL = 1;

#define PIN_OLCL 2
#define PIN_PWM 6
#define PIN_REDLED 7
#define PIN_YELLED 8
#define PIN_V1 A1
#define PIN_V2 A2
#define PIN_V3 A3
#define PIN_RLY1 5
#define PIN_RLY2 4
#define PIN_RLY3 9
#define PIN_VA A0

//#define PIN_PV A0  V_PD = analogRead(A1)*4.096/1.03 * 2.699;

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
unsigned int sec_count = 0; // record curve once every 120 seconds (2 minutes)

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

// PV Panel Limits
float I_in;
float current_limit = 230;
float V_limit = 5000;
float v0, v1; // current and previous voltage values
float p0, p1; // current and previous power values
float i0, i1; // current and previous current values
float p_diff, v_diff;
float V_PD = 0;

// Series Batteries Variables
float V_1 = 0, V_2 = 0, V_3 =0;
float V_UPLIM = 3590;
float V_LOWLIM = 2500;

// State Machine Stuff
boolean input_switch;
int state_num = 0,next_state;
bool started_discharge = 0;
bool stop = 0;
int error1 = 0, error2 = 0, error3 = 0; // 0 for no error, 1 for overcharge, 2 for undercharge, 3 for overheating

// Blinking LED for state 6
boolean blink = 0;

int SoC_1 = 0, SoC_2 = 0, SoC_3 = 0; // Use SoC for balancing
int SoH_1 = 100, SoH_2 = 100, SoH_3 = 100; // SoH
bool relay_on = 0;

// From command reception
int cmd;
int speed; //PWM
int pos_x;
int pos_y;

void setup() {

  //Some General Setup Stuff
  Wire.begin(); // We need this for the i2c comms for the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  ina219.init(); // this initiates the current sensor
  Serial.begin(9600); // USB Communications

  mySMPS.init();

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

  // TimerA0 initialization for 200Hz control-loop interrupt.
  TCA0.SINGLE.PER = 4999; //
  TCA0.SINGLE.CMP1 = 4999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm;

  // TimerB0 initialization for PWM output
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz

  interrupts();  //enable interrupts.
  analogWrite(6, 120); //just a default state to start with

  // Control Iinterface
  ci.setBaudrate(115200);
  ci.setTimeout(5);
  ci.begin();
  delay(3000);
  ci.flushReadBuffer();
  ci.writeSOH(0, mySMPS.get_SOH(1));
  ci.writeSOH(1, mySMPS.get_SOH(2));
  ci.writeSOH(2, mySMPS.get_SOH(3));
}

void loop() {

    //NOTE: command reception loop
    if(ci.fetchData())  {
      cmd = ci.getCommand();
      speed = ci.getSpeed();
      pos_x = ci.getPositionX();
      pos_y = ci.getPositionY();
      Serial.println("Command Received");

      //TODO: need to implement drive status via GPIO connection
      mySMPS.decode_command(cmd, speed, pos_x, pos_y, 1, V_1, V_2, V_3);

      if (mySMPS.recalibrating == 0 && mySMPS.error == 0) {
        next_state = mySMPS.get_state();
        if (cmd == 1) {
           mySMPS.clear_lookup(); // reset tables
        }
      } else if (cmd == 2) {
        next_state = IDLE;
        mySMPS.command_running = 0;
      }

      // If in recalibration, do not halt recalibration
      if (mySMPS.recalibrating == 0) {
        next_state = mySMPS.get_state();
      }
  }

//In rests state, if command was running (but not calibration), command complete
  if (loop_trigger == 1){ // FAST LOOP (currently 200HZ)
      state_num = next_state; //state transition
      ci.writeState(state_num);
      
      //Checking for Error states (individual battery voltages defined)
      if ((V_1 > 3700 || V_1 < 2400) || (V_2 > 3700 || V_2 < 2400) || (V_3 > 3700 || V_3 < 2400)) { 
          state_num = ERROR; //go directly to jail
          next_state = ERROR; // stay in jail
          digitalWrite(PIN_REDLED,true); //turn on the red LED
          current_ref = 0; // no current
          // Error status
          if (V_1 > 3700) {
            error1 = 1;
          } else if (V_1 < 2400) {
            error1 = 2;
          } else {
            error1 = 0;
          }
          if (V_2> 3700) {
            error2 = 1;
          } else if (V_2 < 2400) {
            error2 = 2;
          } else {
            error2 = 0;
          }
          if (V_3> 3700) {
            error3 = 1;
          } else if (V_3 < 2400) {
            error3 = 2;
          } else {
            error3 = 0;
          }
      } else {
        error1 = 0;
        error2 = 0;
        error3 = 0;
      }

      V_PD = analogRead(PIN_VA)*4.096/1.03* 4.1626; //mannual correction for potential divider
      current_measure = (ina219.getCurrent_mA()); // sample the inductor current (via the sensor chip)
     
      if (current_ref == 2000) {
        // do not use PID voltage or current controllers.
      } else if (vref == 3600) { //CONSTANT VOLTAGE CHARGING
        ev = (vref - V_1)/1000.0; 
        cv = pidv(ev); 
        cv = saturation(cv, 0.25, 0); 
        ei = (cv - current_measure)/1000.0; ;
        pwm_out = pidi(ei); 
      } else if (vref == 0) { // Use Current PID controller in all other scenarios
        error_amps = (current_ref - current_measure) / 1000.0;
        pwm_out = pidi(error_amps);
      }
      pwm_out = saturation(pwm_out, 0.99, 0.01); //duty_cycle saturation.
      analogWrite(PIN_PWM, (int)(255 - pwm_out * 255)); // write it out (inverting for the Buck here)
      int_count++; //count how many interrupts since this was last reset to zero
      rly_timer++;
      loop_trigger = 0; //reset the trigger and move on with life
  
  }
  
  // Relay timer is reset every second. Like int_count, it also increments per millisecond.
  // Only switch on relay 1 time per second, and switch on them consecutively
  if (rly_timer == 100) { // Relay switching is 10ms. Double for safety
    digitalWrite(PIN_RLY1,true);
  } else if (rly_timer == 104) { // Read battery 1 voltage
    V_1 = analogRead(A1)*4.096/1.03;
  } else if (rly_timer == 106) {
    digitalWrite(PIN_RLY1,false);
  } else if (rly_timer == 110) {
    digitalWrite(PIN_RLY2,true);
  } else if (rly_timer == 114) {
    V_2 = analogRead(A2)*4.096/1.03;
  } else if (rly_timer == 116) {
    digitalWrite(PIN_RLY2,false);
  } else if (rly_timer == 120) {
    digitalWrite(PIN_RLY3,true);
  } else if (rly_timer == 124) {
    V_3 = analogRead(A3)*4.096/1.03;
  } else if (rly_timer == 126) {
    digitalWrite(PIN_RLY3,false);
    relay_on = 1;
  }
 
  // This still runs every second
  if (int_count == 200) { // SLOW LOOP (1Hz)
    input_switch = digitalRead(PIN_OLCL); //get the OL/CL switch status
    switch (state_num) { // STATE MACHINE (see diagram)
      case IDLE:{ // 0 Idle state (no current, no LEDs)
        current_ref = mySMPS.get_discharge_current();
        next_state = IDLE;
        digitalWrite(PIN_YELLED,false);
        break;
      }
      case CHARGE:{ // 1 Charge state (250mA and a green LED)
        current_ref = 2000; // DO NOT USE PID CONTROLLER

        //NOTE: Instead of using wall charging, use PV charging
        // Assign values
        v0 = V_PD;
        i0 = current_measure; //NOTE: use current output as proxy indicator current input
        
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
            mySMPS.charge_balance(V_1, V_2,V_3, current_measure);            
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
        if(mySMPS.command_running == 1 && mySMPS.recalibrating == 0){
          next_state = IDLE;
          rest_timer = 0;
          digitalWrite(PIN_YELLED,false);
          mySMPS.command_running = 0;
        }
        if (mySMPS.recalibrating == 1) {
            if (rest_timer < 30) { // Stay here if timer < 30
                next_state = CHARGE_REST;
                digitalWrite(PIN_YELLED,false);
                rest_timer++;
            } else { // Move to completion state if battery has already been discharged
                if (started_discharge == 1) {
                    next_state = RECAL_DONE;
                    digitalWrite(PIN_YELLED,false);
                    rest_timer = 0;
                    started_discharge = 0;
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
            mySMPS.discharge_balance(V_1, V_2, V_3, current_measure);                  
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
        if(mySMPS.command_running == 1 && mySMPS.recalibrating == 0){
          next_state = IDLE;
          rest_timer = 0;
          digitalWrite(PIN_YELLED,false);
          mySMPS.command_running = 0;
        }
        if (mySMPS.recalibrating == 1) {
            started_discharge = 1;
            if (rest_timer < 30) { // Rest here for 30s like before
                next_state = DISCHARGE_REST;
                digitalWrite(PIN_YELLED,false);
                rest_timer++;
            } else { // When thats done, move back to charging (and light the green LED)
                mySMPS.send_current_cap(); // coulomb counting during discharge
                next_state = CHARGE;
                digitalWrite(PIN_YELLED,true);
                rest_timer = 0;
            }
        }
        break;
      }
      case ERROR: { // 5 ERROR state RED led and no current
        current_ref = 0;
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
        mySMPS.charge_discharge(current_measure);
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
          mySMPS.recalibrating = 0;
          started_discharge = 0;

          //Evaluate SOC stats
          noInterrupts();
          mySMPS.create_SoC_table();
          interrupts();
          
          SoH_1 = mySMPS.get_SOH(1);
          SoH_2 = mySMPS.get_SOH(2);
          SoH_3 = mySMPS.get_SOH(3);
          dataString = String(2) + "," + String(SoH_1) + "," + String(SoH_2)  + "," + String(SoH_3);
          Serial.println(dataString);
          break;
      }
      case DISCHARGE: { // 8 Normal discharge (-500mA)
        current_ref = mySMPS.get_discharge_current();
         if (V_1 > V_LOWLIM && V_2 > V_LOWLIM && V_3 > V_LOWLIM) { // While not at minimum volts, stay here
            next_state = DISCHARGE;
            digitalWrite(PIN_YELLED,false);
            mySMPS.discharge_balance(V_1, V_2, V_3, current_measure);
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
      default :{ // Should not end up here ....
        Serial.println("Boop");
        current_ref = 0;
        next_state = ERROR; // So if we are here, we go to error
        digitalWrite(PIN_REDLED,true);
        break;
      }    
    }
    
    //NOTE: Evaluate SOC every second, send to control every second
    // SoC Measurement

    if (mySMPS.recalibrating == 0) {
      mySMPS.compute_SOC(state_num, V_1, V_2, V_3);
      SoC_1 = mySMPS.get_SOC(1);
      SoC_2 = mySMPS.get_SOC(2);
      SoC_3 = mySMPS.get_SOC(3);
      ci.writeSOC(1, static_cast<int>(SoC_1));
      ci.writeSOC(2, static_cast<int>(SoC_2));
      ci.writeSOC(3, static_cast<int>(SoC_3));
    }

    ci.sendUpdates();
  
    // Now Print all values to serial and SD
    dataString = String(state_num) + "," + String(V_1) + "," + String(V_2) + "," + String(V_3) + "," + String(current_ref) + "," +String(current_measure) + "," + String(mySMPS.disc1) + "," + String(mySMPS.disc2) + "," + String(mySMPS.disc3);
    Serial.println(dataString);
    
    File dataFile = SD.open("BatCycle.csv", FILE_WRITE);
    if (dataFile){ 
      dataFile.println(dataString);
    } else {
      Serial.println("File not open"); 
    }
    dataFile.close();

    int_count = 0; 
    sec_count++;
  }

  if (sec_count % 5 == 0) {
    rly_timer = 0;
  }
  
  //NOTE - Record curve every 3 minutes, revaluate SoH at the end, build SOC table, and record new charge capacity
  if (sec_count == 180) {
      // OCV: Assume that voltage hasn't drastically changed within past 2 minutes
      // Coulomb counting: charge_diff is adding up the charge (current * time) within the 2 mins
      // adjust for difference when discharge circuit is ON
      if (mySMPS.recalibrating == 1 && started_discharge == 1) {
        mySMPS.record_curve(state_num, V_1, V_2, V_3);
      }
      sec_count = 0;
  }
  
}

// Timer A CMP1 interrupt. Every 1000.0us the program enters this interrupt. This is the fast 1kHz loop
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
