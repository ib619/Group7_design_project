#include <Wire.h>
#include <INA219_WE.h>
#include <SPI.h>
#include <SD.h>
#include <LinkedList.h>

// Package to install: CSV Parser, LinkedList

// initial state of charge for cells (coulombs)
  // Cell 1: 1793
  // Cell 2: 2000.5
  // Cell 3: 1921.75

// Aim:
  // Obtain new charge and discharge curve
  // Compare new discharge coulomb count to original value

// Which cell are you characterising?
int CELL = 1;

INA219_WE ina219; // this is the instantiation of the library for the current sensor

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

const int chipSelect = 10;
unsigned int rest_timer;
unsigned int loop_trigger;
unsigned int int_count = 0; // a variables to count the interrupts. Used for program debugging.
float u0i, u1i, delta_ui, e0i, e1i, e2i; // Internal values for the current controller
float ui_max = 1, ui_min = 0; //anti-windup limitation
float kpi = 0.02512, kii = 39.4, kdi = 0; // current pid.
float Ts = 0.001; //1 kHz control frequency.
float current_measure, current_ref = 0, error_amps; // Current Control
float pwm_out;
float V_Bat;
boolean input_switch;
int state_num=0,next_state;
String dataString;

// Original capacity in terms of charge (hard coded into this script)
float c1_0 = 1793;
float c2_0 = 2000.5;
float c3_0 = 1921.75;
float nominal_cap = 0;
float current_cap = 0;

// Linked Lists for charge voltage, discharge voltage.
LinkedList<float> c_v;
LinkedList<float> d_v;
LinkedList<float> c_SoCList;
LinkedList<float> d_SoCList;
int d_time = 0;
int c_time = 0;
double d_SoC = 1;
double c_SoC = 0;

// File names
String discharge_filename = "DischargeSoC1.csv";
String charge_filename = "ChargeSoC1.csv";
String SoH_filename = "SoHStats1.csv";

// Only start collecting charge data when cell has been discharged.
boolean discharged = 0;

void setup() {
  //Some General Setup Stuff

  Wire.begin(); // We need this for the i2c comms for the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  ina219.init(); // this initiates the current sensor
  Serial.begin(9600); // USB Communications

  //Check for the SD Card
  Serial.println("\nInitializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("* is a card inserted?");
    while (true) {} //It will stick here FOREVER if no SD is in on boot
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // temp.cv simply saves the entire charge cycle for diagnosis, like before
  if (SD.exists("temp.csv")) { 
    SD.remove("temp.csv");
  }
  // Column 1        | Column 2    
  // Entire Timeline | Entire Curve 
  
  // Batcurves stores the discharge and charge curves of the battery
  if (CELL == 1) {
    discharge_filename = "DischargeSoC1.csv";
    charge_filename = "ChargeSoC1.csv";
    SoH_filename = "SoHStats1.csv";
    nominal_cap = c1_0;
  } else if (CELL == 2) {
    discharge_filename = "DischargeSoC2.csv";
    charge_filename = "ChargeSoC2.csv";
    SoH_filename = "SoHStats2.csv";
    nominal_cap = c2_0;
  }  else if (CELL == 3) {
    discharge_filename = "DischargeSoC2.csv";
    charge_filename = "ChargeSoC2.csv";
    SoH_filename = "SoHStats3.csv";
    nominal_cap = c3_0;
  } else {
    Serial.println("\nWe only have 3 cells...");
  }

  if (SD.exists(discharge_filename)) {
    SD.remove(discharge_filename);
  }
  if (SD.exists(charge_filename)) {
    SD.remove(charge_filename);
  }

  // Column 1               | Column 2
  // Discharge/charge curve | SOC

  // Create a new SoHStats.csv if doesn't exist (do nothing)
  // Column 1    | Column 2  
  // Cell_Charge | Cell_SoH 

  
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

  // TimerA0 initialization for 1kHz control-loop interrupt.
  TCA0.SINGLE.PER = 999; //
  TCA0.SINGLE.CMP1 = 999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm;

  // TimerB0 initialization for PWM output
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz

  interrupts();  //enable interrupts.
  analogWrite(6, 120); //just a default state to start with

}

void loop() {
  if (loop_trigger == 1){ // FAST LOOP (1kHZ)
      state_num = next_state; //state transition
      V_Bat = analogRead(A0)*4.096/1.03; //check the battery voltage (1.03 is a correction for measurement error, you need to check this works for you)
      if ((V_Bat > 3700 || V_Bat < 2400)) { //Checking for Error states (just battery voltage for now)
          state_num = 5; //go directly to jail
          next_state = 5; // stay in jail
          digitalWrite(7,true); //turn on the red LED
          current_ref = 0; // no current
      }
      current_measure = (ina219.getCurrent_mA()); // sample the inductor current (via the sensor chip)
      error_amps = (current_ref - current_measure) / 1000; //PID error calculation
      pwm_out = pidi(error_amps); //Perform the PID controller calculation
      pwm_out = saturation(pwm_out, 0.99, 0.01); //duty_cycle saturation
      analogWrite(6, (int)(255 - pwm_out * 255)); // write it out (inverting for the Buck here)
      int_count++; //count how many interrupts since this was last reset to zero
      loop_trigger = 0; //reset the trigger and move on with life
  }
  
  if (int_count == 5000) { // SLOW LOOP (0.2Hz)
    input_switch = digitalRead(2); //get the OL/CL switch status
    switch (state_num) { // STATE MACHINE (see diagram)
      case 0:{ // Start state (no current, no LEDs)
        discharged = 0;
        current_cap = 0;
        current_ref = 0;
        if (input_switch == 1) { // if switch, move to charge
          next_state = 1;
          digitalWrite(8,true);
        } else { // otherwise stay put
          next_state = 0;
          digitalWrite(8,false);
        }
        break;
      }
      case 1:{ // Charge state (250mA and a green LED)
        current_ref = 250;
        if (V_Bat < 3600) { // if not charged, stay put
          next_state = 1;
          digitalWrite(8,true);          
        } else { // otherwise go to charge rest
          next_state = 2;
          digitalWrite(8,false);
        }
        if(input_switch == 0){ // UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(8,false);
        }
        if (discharged == 1) { // Only start recording charge data after complete discharge
          c_v.add(V_Bat);
        }        
        break;
      }
      case 2:{ // Charge Rest, green LED is off and no current
        current_ref = 0;
        if (rest_timer < 30) { // Stay here if timer < 30
          next_state = 2;
          digitalWrite(8,false);
          rest_timer++;           
        } else { // Or move to discharge (and reset the timer)         
          digitalWrite(8,false);
          rest_timer = 0;
          if (discharged == 1){ // Head to save charge data (completion!)
            next_state = 7;
          } else {
            next_state = 3; // Now start discharging 
          }
        }
        if(input_switch == 0){ // UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(8,false);
        }
        break;        
      }
      case 3:{ //Discharge state (-250mA and no LEDs)
         current_ref = -250;
         if (V_Bat > 2500) { // While not at minimum volts, stay here
           next_state = 3;
           digitalWrite(8,false);
         } else { // If we reach full discharged, move to rest
           next_state = 4;
           digitalWrite(8,false);
         }
        if(input_switch == 0){ //UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(8,false);
        }
        d_v.add(V_Bat); // add to discharge list
        current_cap = current_cap + 0.25*5; // FIXME: Adjust for frequency of slow loop
        break;
      }
      case 4:{ // Discharge rest, no LEDs no current
        discharged = 1;
        current_ref = 0;
        if (rest_timer < 30) { // Rest here for 30s like before
          next_state = 4;
          digitalWrite(8,false);
          rest_timer++;
        } else { // When thats done, move back to charging (and light the green LED)
          next_state = 6;
          digitalWrite(8,true);
          rest_timer = 0;
        }
        if(input_switch == 0){ //UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(8,false);
        }
        break;
      }
      case 5: { // ERROR state RED led and no current
        Serial.println("Please restart SoH estimation."); 
        discharged = 0;
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
      case 6: {  // Save discharge data
        d_time = d_v.size()
        
        // Write in first value
        dataString = String(d_SoC) + "," + String(d_v.get(0)); // dSoC is initialised to 1
        Serial.println(dataString);
        File dataFile = SD.open(discharge_filename, FILE_WRITE);
        if (dataFile){
          dataFile.println(dataString);
        } else {
          Serial.println("File not open");
        }
        
        // normalise d_time into a range from 1 to 0 with size d_time, to obtain SoC
        for (int i=0; i < d_time-1; i++) {
          d_SoC = d_SoC - 1/d_time;
          dataString = String(d_SoC) + "," + String(d_v.get(i));
          Serial.println(dataString);
          dataFile.println(dataString);
        } 
        dataFile.close();
        next_state = 1;
      }
      case 7: {  // Save charge data
        c_time = c_v.size()
        
        // Write in first value
        dataString = String(c_SoC) + "," + String(c_v.get(0)); // cSoC is initialised to 0
        Serial.println(dataString);
        File dataFile = SD.open(charge_filename, FILE_WRITE);
        if (dataFile){
          dataFile.println(dataString);
        } else {
          Serial.println("File not open");
        }
        
        // normalise d_time into a range from 1 to 0 with size d_time, to obtain SoC
        for (int i=0; i < c_time-1; i++) {
          c_SoC = c_SoC + 1/c_time;
          dataString = String(c_SoC) + "," + String(c_v.get(i));
          Serial.println(dataString);
          dataFile.println(dataString);
        } 
        dataFile.close();
        next_state = 8;
      }
      case 8: { // Complete!
        // Compare nominal and current capacity
        SoH = current_cap / nominal_cap;

        // Record SoH information
        dataString = String(current_cap) + "," + SoH);
        Serial.println(dataString);
        File dataFile = SD.open(SoH_filename, FILE_WRITE);
        if (dataFile){
          dataFile.println(dataString);
        } else {
          Serial.println("File not open");
        }

        digitalWrite(7,true);
        digitalWrite(8,true);
        next_state = 8;
      }
      default :{ // Should not end up here ....
        Serial.println("Please restart SoH estimation."); 
        Serial.println("Boop");
        current_ref = 0;
        next_state = 5; // So if we are here, we go to error
        digitalWrite(7,true);
      }
      
    }
    
    // Writing to diagnostic file.
    dataString = String(state_num) + "," + String(V_Bat) + "," + String(current_ref) + "," + String(current_measure); //build a datastring for the CSV file
    Serial.println(dataString); // send it to serial as well in case a computer is connected
    File dataFile = SD.open("temp.csv", FILE_WRITE); // open our CSV file
    if (dataFile){ //If we succeeded (usually this fails if the SD card is out)
      dataFile.println(dataString); // print the data
    } else {
      Serial.println("File not open"); //otherwise print an error
    }
    dataFile.close(); // close the file
    int_count = 0; // reset the interrupt count so we dont come back here for 1000ms
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
