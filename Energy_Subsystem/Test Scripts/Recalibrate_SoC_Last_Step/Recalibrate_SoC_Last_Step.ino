/* 

AIM FOR THIS FILE
- Given the array of new cv1,2,3 and dv1,2,3, generate the SoCs and store them in a new file
- no hardware connected

initial state of charge for cells (coulombs)
Cell 1: 1793
Cell 2: 2000.5
Cell 3: 1921.75

Flow chart
typical: 0 > 1 > 6 > 2 > 8 > 4 > 1 > 6 > 2 > 8 > 4 > ......
this file: 0 > 7 > 0

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
#define PIN_REDLED 7
#define PIN_YELLED 8

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

unsigned int loop_trigger;
unsigned int int_count = 0; // a variables to count the interrupts. Used for program debugging.

// Series Batteries Variables

// State Machine Stuff
bool input_switch;
int state_num = 0,next_state, prev_state = -1;

bool recalibrated = 0;

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

  //LEDs on pin 7 and 8
  pinMode(PIN_REDLED, OUTPUT);
  pinMode(PIN_YELLED, OUTPUT);

  // TimerA0 initialization for (NOW) 0.2kHz control-loop interrupt.
  TCA0.SINGLE.PER = 4999; //
  TCA0.SINGLE.CMP1 = 4999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm;

  // TimerB0 initialization for PWM output
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz

  interrupts();  //enable interrupts. 
}

void loop() {

//In rests state, if command was running (but not calibration), command complete
  if (loop_trigger == 1){ // FAST LOOP (0.2kHZ)
      state_num = next_state; //state transition
      int_count++;
      loop_trigger = 0; //reset the trigger and move on with life
  }
 
  // This still runs every 1 second
  if (int_count == 200) { // SLOW LOOP (0.2Hz)
    input_switch = digitalRead(PIN_OLCL); //get the OL/CL switch status
    switch (state_num) { // STATE MACHINE (see diagram)
      case IDLE:{ // 0 Idle state (no current, no LEDs)
        Serial.println("IDLE");
        if (input_switch == 1) {
          next_state = RECAL_DONE;
          digitalWrite(PIN_YELLED,false);
          recalibrated = 0;
        } else { // otherwise stay put
          next_state = IDLE;
          digitalWrite(PIN_YELLED,false);
        }
        break;
      }
      case ERROR: { // 5 ERROR state RED led and no current
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
      case RECAL_DONE: { // 7 Recalibration Complete
        //NOTE: For this test, intentionally the cv_SoC and dv_SoC files first (The purpose is to regenerate these files!)
        Serial.println("RECAL STATE");
        if (recalibrated == 0) {
          if (SD.exists("dv_SoC.csv")) {
                SD.remove("dv_SoC.csv");
           }
           
           if (SD.exists("cv_SoC.csv")) {
                SD.remove("cv_SoC.csv");
           }
           Serial.println("Removed Files");
  
  
          noInterrupts();
          mySMPS.create_SoC_table();
          interrupts();
          
          // Immediately return to IDLE
          Serial.println("Recalibration complete");
          recalibrated = 1;  
        }

        if (input_switch == 0) {
          next_state = IDLE;
        } else {
          next_state = RECAL_DONE;
        }
        
        digitalWrite(PIN_YELLED,false);
        break;
      }
    }
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
