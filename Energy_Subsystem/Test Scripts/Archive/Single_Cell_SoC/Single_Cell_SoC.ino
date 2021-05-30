#include <Wire.h>
#include <INA219_WE.h>
#include <SPI.h>
#include <SD.h>
#include <MovingAverage.h>

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
float V_Bat; float V_PD;
boolean input_switch;
int state_num=0,next_state, prev_state = -1; // -1 prev state for first cycle
String dataString;

float q1_now = 1793;
float SoC_1 = 0;
float temp1 = 0;
float dq1 = 0;
float sum1;
MovingAverage<float> SoC_1_arr = MovingAverage<float>(60);

// SoC Tables
String discharge_SoC_filename = "dv_SoC.csv";
String charge_SoC_filename = "cv_SoC.csv";
float d_v_1[100] = {};
float c_v_1[100] = {};
float d_SoC[100] = {};
float c_SoC[100] = {};
int arr_size = 0;
bool lookup = 0;

// Discharge thresholds
float d_ocv_l_1 = 3150;
float d_ocv_u_1 = 3250;
float c_ocv_u_1 = 3450;
float c_ocv_l_1 = 3300;

void setup() {
  //Some General Setup Stuff

  Wire.begin(); // We need this for the i2c comms for the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  ina219.init(); // this initiates the current sensor
  Serial.begin(9600); // USB Communications

  SoC_1_arr.fill(0);

  //Check for the SD Card
  Serial.println("\nInitializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("* is a card inserted?");
    while (true) {} //It will stick here FOREVER if no SD is in on boot
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  if (SD.exists("BatCycle.csv")) { // Wipe the datalog when starting
    SD.remove("BatCycle.csv");
  }

  // Initialise discharge and charge tables
  File myFile = SD.open(discharge_SoC_filename);
  String content;
  
  if (myFile) {
    Serial.println("Start insertion: discharge");  
      for (int i = 0; i < 100; i++) {
          content = myFile.readStringUntil(',');
          d_v_1[i] = content.toFloat();
          content = myFile.readStringUntil(',');
          // d_v_2[i] = content.toFloat();
          content = myFile.readStringUntil(',');
          // d_v_3[i] = content.toFloat();
          content = myFile.readStringUntil('\n');
          d_SoC[i] = content.toFloat();
          Serial.println(String(d_v_1[i]) + "," + String(d_SoC[i]));
          if (content == "") {
              break;
              Serial.println("Insertion Complete");    
          }                 
      }
  } else {
      Serial.println("File not open");
  }
  myFile.close();

  myFile = SD.open(charge_SoC_filename);
  if (myFile) {
      Serial.println("Start insertion: charge");  
      for (int i = 0; i < 100; i++) {
          content = myFile.readStringUntil(',');
          c_v_1[i] = content.toFloat();
          content = myFile.readStringUntil(',');
          // c_v_2[i] = content.toFloat();
          content = myFile.readStringUntil(',');
          // c_v_3[i] = content.toFloat();
          content = myFile.readStringUntil('\n');
          c_SoC[i] = content.toFloat();
          Serial.println(String(c_v_1[i]) + "," + String(c_SoC[i]));
          if (content == "") {
              break;
              Serial.println("Insertion Complete");    
          }                 
      }
  } else {
      Serial.println("File not open");
  }
  myFile.close();
 
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
      V_Bat = analogRead(A0)*4.096/1.03; //check the battery voltage (1.03 is a correction for measurement error, you need to check this works for you)
      V_PD = analogRead(A1)*4.096/1.03 * 2.699; // obtained via trial and error
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
  
  if (int_count == 200) { // SLOW LOOP (1Hz)
    input_switch = digitalRead(2); //get the OL/CL switch status
    switch (state_num) { // STATE MACHINE (see diagram)
      case 0:{ // Start state (no current, no LEDs)
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
          dq1 = dq1 + current_measure/1000;
          digitalWrite(8,true);          
        } else { // otherwise go to charge rest
          next_state = 2;
          digitalWrite(8,false);
        }
        if(input_switch == 0){ // UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(8,false);
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
          next_state = 3;
          digitalWrite(8,false);
          rest_timer = 0;
        }
        if(input_switch == 0){ // UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(8,false);
        }
        break;        
      }
      case 3:{ //Discharge state (-250mA and no LEDs)
         current_ref = -250;
         dq1 = dq1 + current_measure/1000;
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
        break;
      }
      case 4:{ // Discharge rest, no LEDs no current
        current_ref = 0;
        if (rest_timer < 30) { // Rest here for 30s like before
          next_state = 4;
          digitalWrite(8,false);
          rest_timer++;
        } else { // When thats done, move back to charging (and light the green LED)
          next_state = 1;
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
    // TODO: change to greater equal sign
    temp1 = SoC_1;
    lookup = 1;
    if (state_num == 0 || state_num == 5) { //IDLE
        // LOOKUP for V1, V2, V3
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                temp1 = 0;
                break;
            } else if (V_Bat >= c_v_1[i] && V_Bat < c_v_1[i+1]) {
                temp1 = c_SoC[i];
                break;
            }
        }
    } else if ((state_num == 1) && prev_state == 0) {
      // LOOKUP
      for (int i=0; i < 100; i++) {
            if (i == 99) {
                temp1 = 100;
                break;
            } else if (V_Bat >= c_v_1[i] && V_Bat < c_v_1[i+1]) {
                temp1 = c_SoC[i];
                break;
            }
        }
    } else if ((state_num == 3) && prev_state == 0) {
      for (int i=0; i < 100; i++) {
            if (i == 99) {
                temp1 = 0; //FIXME: need to fix this
                Serial.println("End of table");
                break;
            } else if (V_Bat <= d_v_1[i] && V_Bat > d_v_1[i+1]) {
                temp1 = d_SoC[i];
                Serial.println("Discharge Lookup");
                break;
            }
        }
    } else if (state_num == 1) { // CHARGE
        if (V_Bat > c_ocv_u_1 || V_Bat < c_ocv_l_1) { // LOOKUP        
            for (int i=0; i < 100; i++) {
                if (i == 99) {
                    temp1 = 100;
                    break;
                } else if (V_Bat >= c_v_1[i] && V_Bat < c_v_1[i+1]) {
                    temp1 = c_SoC[i];
                    Serial.println("Charge Lookup");
                    break;
                }
            }
            
        } else { // COULOMB COUNTING
            lookup = 0;
            temp1 = SoC_1 + dq1/q1_now * 100;
            Serial.println("Coulomb Counting");
        }
    } else if (state_num == 3) { // DISCHARGE
        if (V_Bat > d_ocv_u_1 || V_Bat < d_ocv_l_1) { // LOOKUP
            for (int i=0; i < 100; i++) {
                if (i == 99) {
                    temp1 = 0; //FIXME: need to fix this
                    Serial.println("End of table");
                    break;
                } else if (V_Bat <= d_v_1[i] && V_Bat > d_v_1[i+1]) {
                    temp1 = d_SoC[i];
                    Serial.println("Discharge Lookup");
                    break;
                }
            }
        } else { // COULOMB COUNTING
            lookup = 0;
            temp1 = SoC_1 + dq1/q1_now * 100;
            Serial.println("Coulomb Counting");
        }
    } else if (state_num == 2) {
        temp1 = 100;
    } else if (state_num == 4) {
        temp1 = 0;  
    }
    
    // Only perform moving average when voltage lookup is used
    Serial.println(temp1);
    if (arr_size < 60) { // If Moving average filter is not full yet
      sum1 = 0;
      if (arr_size > 5) { // Should ignore first 5 values
        SoC_1_arr.push(temp1);
        if (lookup == 1) {
          for (int i = 0; i < arr_size + 1 - 5; i++) {
             sum1 = sum1 + SoC_1_arr[i];
          }
          SoC_1 = sum1/(arr_size + 1 - 5);
        } else {
          SoC_1 = temp1;
          SoC_1_arr.push(temp1); // just push into FIFO, but not taking the moving average value
          Serial.println("Not moving average");
        }      
      } else {
        SoC_1 = temp1;
      }
      arr_size = arr_size + 1;     
    } else { // In most cases
      if (lookup == 1) {
        SoC_1 = SoC_1_arr.push(temp1).get();
      } else {
        SoC_1 = temp1;
        SoC_1_arr.push(temp1); // just push into FIFO, but not taking the moving average value
        Serial.println("Not moving average");
      }       
    }
    
  
     //CALCULATE GROSS SOC
    prev_state = state_num;
    
    dataString = String(state_num) + "," + String(V_Bat) + "," + String(SoC_1) + "," + String(current_ref) + "," + String(current_measure) + "," + String(V_PD); //build a datastring for the CSV file
    Serial.println(dataString); // send it to serial as well in case a computer is connected
    File dataFile = SD.open("BatCycle.csv", FILE_WRITE); // open our CSV file
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
