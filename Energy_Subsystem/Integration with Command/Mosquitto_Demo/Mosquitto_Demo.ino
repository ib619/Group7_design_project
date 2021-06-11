 /* 

ATTEMPT TO IMPLEMENT PnO Algorithm for MPPT
V/I Limit: 5V, 230mA
Perturb and Observe Algorithm
LED is ON when sweeping.
Switch on Lamp!!!

Sweep IV curve: From 0 to -230mA. Stay at each current for 3 seconds
0: IDLE
1: Charge with PNO
2: Charge Complete (never)

How bout we try to control the current on the output side?
  Panel connected to V_B
  120R + 75R resistor connected to A
  Synchronous

Final test: 
  asynchronous boost J1 left, J2 right
  asynchronous buck J1 right, J2 left
*/
  

//Packages
#include <Wire.h>
#include <INA219_WE.h>
#include <SPI.h>
#include <SD.h>
INA219_WE ina219; // this is the instantiation of the library for the current sensor

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 10; // SD

// rest timer during charge and discharge rest
unsigned int rest_timer;

// triggers the fast (1kHz) loop. int_count = 1000 runs slow (1Hz) loop.
unsigned int loop_trigger;
unsigned int int_count = 0; // a variables to count the interrupts. Used for program debugging. 

// Voltage PID Controller Stuff
float ev=0,cv=0,ei=0; //internal signals // FIXME:
  // ev: difference between V_ref and V_b
  // cv: current obtained from voltage PID controller. need to saturate it
  // ei: difference between desired and inductor current. error_amps in this case? FIXME:
float kpv=0.05024,kiv=15.78,kdv=0; // voltage pid.
float u0v,u1v,delta_uv,e0v,e1v,e2v; // u->output; e->error; 0->this time; 1->last time; 2->last last time
float uv_max=4, uv_min=0; //anti-windup limitation

// Current PID Controller Stuff
float u0i, u1i, delta_ui, e0i, e1i, e2i; // Internal values for the current controller
float ui_max = 1, ui_min = 0; //anti-windup limitation
float kpi = 0.02512, kii = 39.4, kdi = 0; // current pid.
float Ts = 0.001; //1 kHz control frequency.

float current_measure; // obtained from ina219 (inductor current)
float current_ref = 0, error_amps; // Current Control
float pwm_out;
double V_B; // voltage at terminal VB
double V_A; // voltage at terminal VA
boolean input_switch; // OLCL switch. 0 means back to IDLE

bool move_on = 0;

// PV Panels PnO Algorithm
float current_limit = +230;
float V_limit = 4700;
float v0, v1; // current and previous voltage values
float p0, p1; // current and previous power values
float i0, i1; // current and previous current values
float p_diff, v_diff;

// State Machine
int state_num=0,next_state;
String dataString;

void setup() {
  //Some General Setup Stuff

  Wire.begin(); // We need this for the i2c comms for the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  ina219.init(); // this initiates the current sensor
  Serial.begin(9600); // USB Communications

  //Check for the SD Card - Initiate a "PVPWMSwe.csv" upon reset
  Serial.println("\nInitializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("* is a card inserted?");
    while (true) {} //It will stick here FOREVER if no SD is in on boot
  } else {
    Serial.println("Here comes the sun and I say it's all right!");
    Serial.println("Printing state_num, V_panels, current_measure, V_resistors, P_panels");
  }
  if (SD.exists("PVPWMSwe.csv")) { // Wipe the datalog when starting
    SD.remove("PVPWMSwe.csv");
  }
  
  noInterrupts(); //disable all interrupts
  analogReference(EXTERNAL); // We are using an external analogue reference for the ADC

  //SMPS Pins
  pinMode(13, OUTPUT); // Using the LED on Pin D13 to indicate status

  // TODO: Reassign these pins
  pinMode(2, INPUT_PULLUP); // Pin 2 is the input from the CL/OL switch
  pinMode(6, OUTPUT); // This is the PWM Pin

  //LEDs on pin 7 and 8
  pinMode(7, OUTPUT); // Error: Red Light
  pinMode(8, OUTPUT); // Performing PnO Algorithm: Yellow

  //Analogue input, currently the battery voltage (also port B voltage)
  // TODO: Do we want to connect the solar PV panel in boost or or in Buck? Boost would be more convenient
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

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

  // FAST LOOP (1kHZ)
  if (loop_trigger == 1){
      state_num = next_state; //state transition
      V_B = analogRead(A0)*4.096/1.03* 1.481816;
      V_A = analogRead(A1)*4.096/1.03* 4.142073;
      if (V_B > 21000) { //Checking for Error states (just battery voltage for now) //TODO: adjust value for one PV panel
          state_num = 5; //go directly to jail
          next_state = 5; // stay in jail
          digitalWrite(7,true); //turn on the red LED
      }
      current_measure = (ina219.getCurrent_mA()); // sample the inductor current (via the sensor chip)
      pwm_out = saturation(pwm_out, 0.99, 0.01);  //duty_cycle saturation
      analogWrite(6, (int)(255 - pwm_out * 255)); // write it out (inverting for the Buck here) //Not TODO: PWM Modulate Function

      // Update Flags
      int_count++; //count how many interrupts since this was last reset to zero
      loop_trigger = 0; //reset the trigger and move on with life
  }
  
  // SLOW LOOP (2 seconds)
  if (int_count == 2000) { 
    input_switch = digitalRead(2); //get the OL/CL switch status
    switch (state_num) { // STATE MACHINE (see diagram)
      case 0:{ // Start state (no current, no LEDs)
        if (input_switch == 1) {
          // First time, so reset voltage panel values
          pwm_out = 0.5; // initial pwm value   
          v1 = V_B;
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
      case 1:{ // Running state (a green LED)
        // Assign values
        v0 = V_B;
        i0 = current_measure;
        
        p0 = v0 * i0; // directly use I_out as a proxy indicator for I_in
        p_diff = p0-p1;       
        v_diff = v0-v1;

        // in general increasing PWM means decreasing PV voltage
        // PnO algorithm
        if (((p0>p1) && (v0>v1) || (p0<p1) && (v0<v1))) {
          pwm_out = pwm_out - 0.03;
        } else if ((p0<p1) && (v0>v1) || (p0>p1) && (v0<v1)) {
          pwm_out = pwm_out + 0.03;
        } else {
          Serial.println("No increment");
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
      case 2: {
        if(input_switch == 0){ // UNLESS the switch = 0, then go back to start
          next_state = 0;
          digitalWrite(8,false);
        } else {
          digitalWrite(8,true);
          digitalWrite(7,true);
          next_state = 0;
        }
      }
      case 5: { // ERROR state RED led and no current
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
        Serial.println("Boob");
        next_state = 5; // So if we are here, we go to error
        digitalWrite(7,true);
      }
      
    }
    
    // Print values to serial monitor and csv
    // State number, PV Voltage Reference(V), PV Voltage(V), PV Power(W)
    dataString = String(state_num) + "," + String(pwm_out) + "," + String(V_B) + ","+ String(current_measure) + "," + String(V_A) + "," + String(V_B*current_measure/1000000); //build a datastring for the CSV file
    Serial.println(dataString); // send it to serial as well in case a computer is connected

    File dataFile = SD.open("PVPWMSwe.csv", FILE_WRITE);
    if (dataFile){
      dataFile.println(dataString);
    } else {
      Serial.println("File not open"); 
    }
    dataFile.close();
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
