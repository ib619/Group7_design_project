#include "Power.h"
#include <Arduino.h>

/*

Flow chart (typical)
----------------------
0 > 1 > 2 > 3 > 4 > 1 > 2 > 3 > 4 > ......

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
11 AFTER DISCHARGING AND CHARGING (then send voltage curves and current curves)


Summary of SD csv files
-----------------------
Stats.csv: q1,q2,qq3_now
cv_SoC.csv: V1 V2 V3 SOC
dv_SoC.csv: V1 V2 V3 SOC
thresholds.csv: d_ocv_l_1, d_ocv_u_1, c_ocv_u_1, c_ocv_l_1, (then same for cell 2 and 3, on the same row)


Frequency of recording
----------------------
Recalibration - manual
    Record new discharge and charge voltage values - every 2 minutes
    End of recalibration: send SoH values, as well as current capacity (in terms of charge)
Send SoC info - every half minute


*/

SMPS::SMPS() {};

// NOTE: Also need to load in threshold values
void SMPS::init() {

    /* Need to Initialise
        Current statistics: current maximal charge, determined from last calibration
        Charge discharge curves, from SD
        Threshold voltages
    */

   SoC_1_arr.fill(0);
   SoC_2_arr.fill(0);
   SoC_3_arr.fill(0);

    //Check for the SD Card
    Serial.println("\nInitializing SD card...");
    if (!SD.begin(SD_CS)) {
        Serial.println("* is a card inserted?");
        while (true) {} // Stick here FOREVER if no SD is in on boot
    } else {
        Serial.println("Wiring is correct and a card is present.");
    }

    String content;

    // Current maximal charge, determined from last calibration
    // 3 items in one row: q1_now, q2_now, q3_now. Determins SoC by lookup upon booting up.
    if (SD.exists("Stats.csv")) {
        myFile = SD.open("Stats.csv");  
        if(myFile) {   
            Serial.println("Stats file open");    
            for (int i = 0; i < 2; i++) {
                content = myFile.readStringUntil(',');
                Serial.println(content);
                if (i == 0) {
                    q1_now = content.toFloat();
                }
                if (i == 1) {
                    q2_now = content.toFloat();
                }
            }
            // Finally
            content = myFile.readStringUntil('\n');
            q3_now = content.toFloat();
        }
    } else {
        Serial.println("File not open");
    }
    myFile.close();
    SoH_1 = q1_now/q1_0*100;
    SoH_2 = q2_now/q2_0*100;
    SoH_3 = q3_now/q3_0*100;

    // Initialise discharge and charge tables
    if (SD.exists(discharge_SoC_filename)) {
        myFile = SD.open(discharge_SoC_filename);
        if (myFile) {
            for (int i = 0; i < 100; i++) {
                content = myFile.readStringUntil(',');
                d_v_1[i] = content.toFloat();
                content = myFile.readStringUntil(',');
                d_v_2[i] = content.toFloat();
                content = myFile.readStringUntil(',');
                d_v_3[i] = content.toFloat();
                content = myFile.readStringUntil('\n');
                d_SoC[i] = content.toFloat();
                Serial.println(
                    String(d_v_1[i]) + "," + 
                    String(d_v_2[i]) + "," + 
                    String(d_v_3[i]) + "," + 
                    String(d_SoC[i])
                );
                if (content == "") {
                    Serial.println("Insertion Complete");   
                    break; 
                }                 
            }
        }
    } else {
        Serial.println("File not open");
    }
    myFile.close();

    if (SD.exists(charge_SoC_filename)) {
        myFile = SD.open(charge_SoC_filename);
        if (myFile) {
            for (int i = 0; i < 100; i++) {
                content = myFile.readStringUntil(',');
                c_v_1[i] = content.toFloat();
                content = myFile.readStringUntil(',');
                c_v_2[i] = content.toFloat();
                content = myFile.readStringUntil(',');
                c_v_3[i] = content.toFloat();
                content = myFile.readStringUntil('\n');
                c_SoC[i] = content.toFloat();
                Serial.println(
                    String(c_v_1[i]) + "," 
                    + String(c_v_2[i]) + "," 
                    + String(c_v_3[i])  + "," 
                    + String(c_SoC[i])
                );
                if (content == "") {
                    Serial.println("Insertion Complete");    
                    break;
                    
                }                 
            }
        }
    } else {
        Serial.println("File not open");
    }
    myFile.close();
}

void SMPS::triggerError() {
    state = 5;
    error = 1;
}

//TODO: this can be called even if command is running
void SMPS::reset() {
    error = 0;
    state = 0;
    command_running = 0;
}

int SMPS::get_state() {
    return state;
}

void SMPS::decode_command(int cmd, int speed, int pos_x, int pos_y) {
    if (cmd == 0) {
        if (pos_x == 0 && pos_y == 0) {
            charge(); //FIXME: need to do constant charge afterwards, IDLE
        } else {
            if (speed == 0) {
                stop() // IDLE
            } else {
                discharge()// discharge
            }
        }
    } else if (cmd == 1) {
        recalibrate_SOH();
    }
}

float SMPS::estimate_range(float x0, float y0, bool atCharger) {
    if ((x0 ==0 && y0 == 0) || atCharger == 1) {
        x0 = 0, y0 = 0;
    } else {

    }
}

float SMPS::estimate_time() {
    float gross_Charge = (SoC_1*q1_now + SoC_2*q2_now + SoC_3*q3_now)/100;
    float remaining_Ws = 3.2 * gross_SoC; // Very rough: Discharge curve at around 3.2V most of the time
    
   // Assume average power dissipated is 5.01V x 0.14A = 0.7014W
   float remaining_Time = remaining_Ws/0.7014; // seconds
 
   // Return in Hours
   return remaining_Time * 0.55 / 60 / 24; // From lab results, efficiency of boost at I_in = 250mA is approx 65%
}

float SMPS::estimate_chargeTime() {
    //TODO: Need to characterise charging battery with SMPS and look at its charging curve 
}

void SMPS::charge() {
    command_running = 1;
    state = 1;
    // will then automatically go on to state 6 then 2.
}

void SMPS::rapid_charge() {
    command_running = 1;
    state = 10;
}

void SMPS::discharge() {
    command_running = 1;
    state = 8;
}

void SMPS::rapid_discharge() {
    command_running = 1;
    state = 9;
}

void SMPS::stop() {
    command_running = 0;
    state = 0;
}

void SMPS::recalibrate_SOH() {
    recalibrating = 1;
    c_iterator = 0;
    d_iterator = 0;
}

// should not halt recalibration process
bool SMPS::get_recalibrate() {
    state = 1;
    return recalibrating;
}

// q1, q2, q3 will vary because we might use the discharge to divert some of the current
void SMPS::send_current_cap(float q1, float q2, float q3) {
    q1_now = q1;
    q2_now = q2;
    q3_now = q3;

    SoH_1 = q1_now/q1_0*100;
    SoH_2 = q2_now/q2_0*100;
    SoH_3 = q3_now/q3_0*100;

    //NOTE: Also write to SD card first.
    dataString = String(q1_now) + "," + String(q2_now) + "," + String(q3_now);

    if (SD.exists("Stats.csv")) {
        SD.remove("Stats.csv");
    }

    myFile = SD.open("Stats.csv", FILE_WRITE);
    if (myFile){ 
      myFile.println(dataString); 
    } else {
      Serial.println("File not open"); 
    }
    myFile.close();
}

int SMPS::get_SOH(int cell_num) {
    if (cell_num == 1) {
        return static_cast<int>(SoH_1);
  } else if (cell_num == 2) {
        return static_cast<int>(SoH_2);
  } else if (cell_num == 3) {
        return static_cast<int>(SoH_3);
  }
}

void SMPS::compute_SOC(int state_num, float V_1, float V_2, float V_3, float charge_1, float charge_2, float charge_3) {
    float temp1 = SoC_1;
    float temp2 = SoC_2;
    float temp3 = SoC_3;
    bool lookup = 1;

    if (state_num == 0 && prev_state == -1) {
        temp1 = lookup_c_table(1, V_1, V_2, V_3);
        temp2 = lookup_c_table(2, V_1, V_2, V_3);
        temp3 = lookup_c_table(3, V_1, V_2, V_3);
        Serial.println("Start log");
    } else if (state_num == 0 || state_num == 5 || state_num == 7) { //IDLE
        // LOOKUP for V1, V2, V3
        // FREEZE values, do nothing
    } else if ((state_num == 1 || state_num == 6 || state_num == 10) && prev_state == 0){ // starting Charge
         // LOOKUP for V1, V2, V3
        temp1 = lookup_c_table(1, V_1, V_2, V_3);
        temp2 = lookup_c_table(2, V_1, V_2, V_3);
        temp3 = lookup_c_table(3, V_1, V_2, V_3);
        lookup = 0;
    } else if ((state_num == 3 || state_num == 8 || state_num == 9) && prev_state == 0){ // starting discharge
         // LOOKUP for V1, V2, V3
        temp1 = lookup_d_table(1, V_1, V_2, V_3);
        temp2 = lookup_d_table(2, V_1, V_2, V_3);
        temp3 = lookup_d_table(3, V_1, V_2, V_3);
        lookup = 0;
    } else if (state_num == 1 || state_num == 6 || state_num == 10) { // CHARGE
        if (V_1 > c_ocv_u || V_1 < c_ocv_l) { // LOOKUP        
            temp1 = lookup_c_table(1, V_1, V_2, V_3); 
        } else { // COULOMB COUNTING
            temp1 = temp1 + charge_1/q1_now*100;
            lookup = 0;
        }
        if (V_2 > c_ocv_u || V_2 < c_ocv_l) { // LOOKUP
            temp2 = lookup_c_table(2, V_1, V_2, V_3);
        } else { // COULOMB COUNTING  
            temp2 = temp2 + charge_2/q2_now*100;
            lookup = 0;
        }
        if (V_3 > c_ocv_u || V_3 < c_ocv_l) { // LOOKUP  
            temp3 = lookup_c_table(3, V_1, V_2, V_3);          
        } else { // COULOMB COUNTING
            temp3 = temp3 + charge_3/q3_now*100;
            lookup = 0;
        }
    } else if (state_num == 3 || state_num == 8 || state_num == 9) { // DISCHARGE
        if (V_1 > d_ocv_u || V_1 < d_ocv_l) { // LOOKUP
            temp1 = lookup_d_table(1, V_1, V_2, V_3);
        } else { // COULOMB COUNTING
            temp1 = temp1 + charge_1/q1_now*100;
            lookup = 0;
        }
        if (V_2 > d_ocv_u || V_2 < d_ocv_l) { // LOOKUP           
            temp2 = lookup_d_table(2, V_1, V_2, V_3);
        } else { // COULOMB COUNTING
            temp2 = temp2 + charge_2/q2_now*100;
            lookup = 0;
        }
        if (V_3 > d_ocv_u || V_3 < d_ocv_l) { // LOOKUP
            temp3 = lookup_d_table(3, V_1, V_2, V_3);
        } else { // COULOMB COUNTING
            temp3 = temp3 + charge_3/q3_now*100;
            lookup = 0;
        }
    } else if (state_num == 2) {
        temp1 = 100;
        temp2 = 100;
        temp3 = 100;
    } else if (state_num == 4) {
        temp1 = 0;
        temp2 = 0;
        temp3 = 0;
    }

    if (arr_size < 60) { // If Moving average filter is not full yet
      float sum1 = 0, sum2 = 0, sum3 = 0;
      if (arr_size > 5) { // Should ignore first 5 values
        SoC_1_arr.push(temp1);
        SoC_2_arr.push(temp2);
        SoC_3_arr.push(temp3);
        if (lookup == 1) {
          for (int i = 0; i < arr_size + 1 - 5; i++) {
             sum1 = sum1 + SoC_1_arr[i];
             sum2 = sum2 + SoC_2_arr[i];
             sum3 = sum3 + SoC_3_arr[i];
          }
          SoC_1 = sum1/(arr_size + 1 - 5);
          SoC_2 = sum2/(arr_size + 1 - 5);
          SoC_3 = sum3/(arr_size + 1 - 5);
        } else { // Do not get moving average filter value for colomb count
          SoC_1 = temp1;
          SoC_2 = temp2;
          SoC_3 = temp3;
          Serial.println("Not moving average");
        }      
      } else { // Do not push into filter for first 5 values
        SoC_1 = temp1;
        SoC_2 = temp2;
        SoC_3 = temp3;
      }
      arr_size = arr_size + 1;     
    } else { // In most cases
      if (lookup == 1) {
        SoC_1 = SoC_1_arr.push(temp1).get();
        SoC_2 = SoC_2_arr.push(temp2).get();
        SoC_3 = SoC_3_arr.push(temp3).get();
      } else {
        SoC_1 = temp1;
        SoC_2 = temp2;
        SoC_3 = temp3;
        SoC_1_arr.push(temp1); // just push into FIFO, but not taking the moving average value
        SoC_2_arr.push(temp2);
        SoC_3_arr.push(temp3);
        Serial.println("Not moving average");
      }       
    }

     //Assign previous state
    prev_state = state_num;

    // Now Print all values to serial and SD
    dataString = String(state_num) + "," + String(V_1) + "," + String(V_2) + "," + String(V_3) + "," + String(SoC_1) + "," + String(SoC_2)  + "," + String(SoC_3)  + "," + String(q1_now) + "," + String(q2_now)  + "," + String(q3_now);
    Serial.println(dataString);

    myFile = SD.open("Diagnose.csv", FILE_WRITE);
    if (myFile){ 
      myFile.println(dataString);
    } else {
      Serial.println("File not open"); 
    }
    myFile.close();
}

float SMPS::get_SOC(int cell_num) {
  if (cell_num == 1) {
    return SoC_1;
  } else if (cell_num == 2) {
    return SoC_2;
  } else if (cell_num == 3) {
    return SoC_3;
  }
}

float SMPS::lookup_c_table(int cell_num, float V_1, float V_2, float V_3) {
    if (cell_num == 1) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 1.0;
            } else if (V_1 >= c_v_1[i] && V_1 < c_v_1[i+1]) {
                Serial.println("Looked up charge table");
                return c_SoC[i];
            }
        }
    } else if (cell_num == 2) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 1.0;
            } else if (V_2 >= c_v_2[i] && V_2 < c_v_2[i+1]) {
                Serial.println("Looked up charge table");
                return c_SoC[i];
            }
        }
    } else if (cell_num == 3) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 1.0;
            } else if (V_3 >= c_v_3[i] && V_3 < c_v_3[i+1]) {
                Serial.println("Looked up charge table");
                return c_SoC[i];
            }
        }
    }  
}

float SMPS::lookup_d_table(int cell_num, float V_1, float V_2, float V_3) {
    //NOTE: changed to less than equal to operator
    if (cell_num == 1) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 0.0;
            } else if (V_1 <= d_v_1[i] && V_1 > d_v_1[i+1]) {
                Serial.println("Looked up discharge table");
                return d_SoC[i];
            }
        }
    } else if (cell_num == 2) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 0.0;
            } else if (V_2 <= d_v_2[i] && V_2 > d_v_2[i+1]) {
                Serial.println("Looked up discharge table");
                return d_SoC[i];
            }
        }
    } else if (cell_num == 3) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 0.0;
            } else if (V_3 <= d_v_3[i] && V_3 > d_v_3[i+1]) {
                Serial.println("Looked up discharge table");
                return d_SoC[i];
            }
        }
    }
}

void SMPS::clear_lookup() {
    // Clear lookup table on SD
    if (SD.exists(discharge_SoC_filename)) {
        SD.remove(discharge_SoC_filename);
    }
    if (SD.exists(charge_SoC_filename)) {
        SD.remove(charge_SoC_filename);
    }

    // Erase Tables
    memset(d_v_1, 0, sizeof(d_v_1));
    memset(c_v_1, 0, sizeof(c_v_1));
    memset(d_v_2, 0, sizeof(d_v_2));
    memset(c_v_2, 0, sizeof(c_v_2));
    memset(d_v_3, 0, sizeof(d_v_3));
    memset(c_v_3, 0, sizeof(c_v_3));
}

//TODO: clear tables before recording curves (before recalibration)
void SMPS::record_curve(int state_num, float V_1, float V_2, float V_3) {
    // Decide thresholds after evaluating entire table
    dataString = String(V_1) + "," + String(V_2) + "," + String(V_3);

    if (state_num == 1 || state_num == 2 || state_num == 6) {
        myFile = SD.open("cv.csv", FILE_WRITE); // only for backup
        c_v_1[c_iterator] = V_1;
        c_v_2[c_iterator] = V_2;
        c_v_3[c_iterator] = V_3;
    } else if (state_num == 3 || state_num == 4) {
        myFile = SD.open("dv.csv", FILE_WRITE); // only for backup
        d_v_1[d_iterator] = V_1;
        d_v_2[d_iterator] = V_2;
        d_v_3[d_iterator] = V_3;
    }

    if (myFile) {
        myFile.println(dataString);
    }
    myFile.close();
    d_iterator = d_iterator + 1;
    c_iterator = c_iterator + 1;
}

void SMPS::create_SoC_table() {
    float d_SoC_1 = 100;
    float c_SoC_1 = 0;

    float d_size = static_cast<float>(d_iterator);
    float c_size = static_cast<float>(c_iterator);

    // Erase Tables
    memset(d_SoC, 0, sizeof(d_SoC));
    memset(c_SoC, 0, sizeof(c_SoC));

    //Need to erase file first. Not the same for SoH file, which keeps the entire battery history
    if (SD.exists(discharge_SoC_filename)) {
        SD.remove(discharge_SoC_filename);
    }
    myFile = SD.open(discharge_SoC_filename, FILE_WRITE);
    for(int i = 0; i < d_iterator; i++){
        if (i == d_iterator - 1) {
            dataString = String(d_v_1[i]) + "," + String(d_v_2[i]) + "," + String(d_v_3[i]) + "," + String(0);
        } else {
            dataString = String(d_v_1[i]) + "," + String(d_v_2[i]) + "," + String(d_v_3[i]) + "," + String(d_SoC_1);
        }    
        // Serial.println(dataString);
        myFile.println(dataString);
        d_SoC[i] = d_SoC_1; // insert value into array
        d_SoC_1 = d_SoC_1 - 1/d_size;
    }
    myFile.close();

    if (SD.exists(charge_SoC_filename)) {
        SD.remove(charge_SoC_filename);
    }
    myFile = SD.open(charge_SoC_filename, FILE_WRITE);
    for(int i = 0; i < c_iterator; i++){
        if (i == c_iterator - 1) {
            dataString = String(c_v_1[i]) + "," + String(c_v_2[i]) + "," + String(c_v_3[i]) + "," + String(1);
        } else {
            dataString = String(c_v_1[i]) + "," + String(c_v_2[i]) + "," + String(c_v_3[i]) + "," + String(c_SoC_1);      
        }
        // Serial.println(dataString);
        myFile.println(dataString);
        c_SoC[i] = c_SoC_1; // insert value into array
        c_SoC_1 = c_SoC_1 + 1/c_size;
    }
    myFile.close();

}

//NOTE: Save thresholds onto Stats.csv
float SMPS::determine_cv_threshold() {
    int iterator = 0;
    for (int i = 0; i < c_iterator-1; i++) {
        if (c_v_1[i+1] - c_v_1[i] < 0.12 * 120 && c_v_1[i] < 3300) {
            c_ocv_l_1 = c_v_1[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < c_iterator-1; i++) {
        if (c_v_1[i+1] - c_v_1[i] > 0.1 * 120 && c_v_1[i] > 3400) {
            c_ocv_u_1 = c_v_1[i];          
            break;
        }
    }

    iterator = 0;
    for (int i = 0; i < c_iterator-1; i++) {
        if (c_v_2[i+1] - c_v_2[i] < 0.12 * 120 && c_v_2[i] < 3300) {
            c_ocv_l_2 = c_v_2[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < c_iterator-1; i++) {
        if (c_v_2[i+1] - c_v_2[i] > 0.1 * 120 && c_v_2[i] > 3400) {
            c_ocv_u_2 = c_v_2[i];          
            break;
        }
    }

    iterator = 0;
    for (int i = 0; i < c_iterator-1; i++) {
        if (c_v_3[i+1] - c_v_3[i] < 0.12 * 120 && c_v_3[i] < 3300) {
            c_ocv_l_3 = c_v_3[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < c_iterator-1; i++) {
        if (c_v_3[i+1] - c_v_3[i] > 0.1 * 120 && c_v_3[i] > 3400) {
            c_ocv_u_3 = c_v_3[i];          
            break;
        }
    }
}

float SMPS::determine_dv_threshold() {
    int iterator = 0;
    for (int i = 0; i < d_iterator-1; i++) {
        if (d_v_1[i+1] - d_v_1[i] > -0.12 * 120 && d_v_1[i] > 3200) {
            d_ocv_u_1 = d_v_1[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < d_iterator-1; i++) {
        if (d_v_1[i+1] - d_v_1[i] < - 0.08 * 120 && d_v_1[i] < 3100) {
            d_ocv_u_1 = d_v_1[i];          
            break;
        }
    }
    
    iterator = 0;
    for (int i = 0; i < d_iterator-1; i++) {
        if (d_v_2[i+1] - d_v_2[i] > -0.12 * 120 && d_v_2[i] > 3200) {
            d_ocv_u_2 = d_v_2[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < d_iterator-1; i++) {
        if (d_v_2[i+1] - d_v_2[i] < - 0.08 * 120 && d_v_2[i] < 3100) {
            d_ocv_u_2 = d_v_2[i];          
            break;
        }
    }

    iterator = 0;
    for (int i = 0; i < d_iterator-1; i++) {
        if (d_v_3[i+1] - d_v_3[i] > -0.12 * 120 && d_v_3[i] > 3200) {
            d_ocv_u_3 = d_v_3[i];
            break;
        }
    }
    for (int i = iterator; i < d_iterator-1; i++) {
        if (d_v_3[i+1] - d_v_3[i] < - 0.08 * 120 && d_v_3[i] < 3100) {
            d_ocv_u_3 = d_v_3[i];          
            break;
        }
    }
}
