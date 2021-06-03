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
   // SoC_2_arr.fill(0);
   //  SoC_3_arr.fill(0);

    //Check for the SD Card
    Serial.println("\nInitializing SD card...");
    if (!SD.begin(SD_CS)) {
        Serial.println("* is a card inserted?");
        while (true) {} // Stick here FOREVER if no SD is in on boot
    } else {
        Serial.println("Wiring is correct and a card is present.");
    }

    String content;

    /*
    // Current maximal charge, determined from last calibration
    // 3 items in one row: q1_now, q2_now, q3_now. Determins SoC by lookup upon booting up.
    if (SD.exists("Stats.csv")) {
        myFile = SD.open("Stats.csv");  
        if(myFile) {   
            Serial.println("Stats file open");    
            for (int i = 0; i < 10; i++) {
                content = (myFile.readStringUntil(',')).toFloat();
                Serial.println(content);
                if (i == 0) {
                    q1_now = content;
                }
                if (i == 1) {
                    q2_now = content;
                }
                if (i == 2) {
                    q3_now = content;
                }
            }
            // Finally
            content = myFile.readStringUntil('\n');
            gross_SoC = content;
        }
    } else {
        Serial.println("File not open");
    }
    myFile.close();
    */
   q1_now = q1_0;

    // Initialise discharge and charge tables
    if (SD.exists(discharge_SoC_filename)) {
        myFile = SD.open(discharge_SoC_filename);
        if (myFile) {
            for (int i = 0; i < 100; i++) {
                content = myFile.readStringUntil(',');
                d_v_1[i] = content.toFloat();
                content = myFile.readStringUntil(',');
                //d_v_2[i] = content.toFloat();
                content = myFile.readStringUntil(',');
                //d_v_3[i] = content.toFloat();
                content = myFile.readStringUntil('\n');
                d_SoC[i] = content.toFloat();
                Serial.println(
                    String(d_v_1[i]) + "," + 
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
                // c_v_2[i] = content.toFloat();
                content = myFile.readStringUntil(',');
                // c_v_3[i] = content.toFloat();
                content = myFile.readStringUntil('\n');
                c_SoC[i] = content.toFloat();
                Serial.println(
                    String(c_v_1[i]) + "," 
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

    /*
    // Load threshold values
    if (SD.exists(threshold_filename)) {
        myFile = SD.open(threshold_filename);
        if (myFile) {
            for (int i = 0; i < 5; i++) {
                content = myFile.readStringUntil(',');
                d_ocv_l_1 = content.toFloat();
                content = myFile.readStringUntil(',');
                d_ocv_u_1 = content.toFloat();
                content = myFile.readStringUntil(',');
                c_ocv_u_1 = content.toFloat();
                content = myFile.readStringUntil(',');
                c_ocv_l_1 = content.toFloat();
                content = myFile.readStringUntil(',');
                d_ocv_l_2 = content.toFloat();
                content = myFile.readStringUntil(',');
                d_ocv_u_2 = content.toFloat();
                content = myFile.readStringUntil(',');
                c_ocv_u_2 = content.toFloat();
                content = myFile.readStringUntil(',');
                c_ocv_l_2 = content.toFloat();
                content = myFile.readStringUntil(',');
                d_ocv_l_3 = content.toFloat();
                content = myFile.readStringUntil(',');
                d_ocv_u_3 = content.toFloat();
                content = myFile.readStringUntil(',');
                c_ocv_u_3 = content.toFloat();
                content = myFile.readStringUntil('\n');
                c_ocv_l_3 = content.toFloat();
                Serial.println(
                    "Cell1_Thresholds: "
                    String(d_ocv_l_1) + "," 
                    + String(d_ocv_u_1) + "," 
                    + String(c_ocv_u_1)  + "," 
                    + String(c_ocv_l_1)
                );
                Serial.println(
                    "Cell2_Thresholds: "
                    String(d_ocv_l_2) + "," 
                    + String(d_ocv_u_2) + "," 
                    + String(c_ocv_u_2)  + "," 
                    + String(c_ocv_l_2)
                );
                Serial.println(
                    "Cell3_Thresholds: "
                    String(d_ocv_l_3) + "," 
                    + String(d_ocv_u_3) + "," 
                    + String(c_ocv_u_3)  + "," 
                    + String(c_ocv_l_3)
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
    */

}

void SMPS::compute_SOC(int state_num, float V_1, float V_2, float V_3, float charge_1, float charge_2, float charge_3) {
    float temp1 = SoC_1;
    bool lookup = 1;

    if (state_num == 0 && prev_state == -1) {
        temp1 = lookup_c_table(1, V_1, V_2, V_3);
        Serial.println("Start log");
    } else if (state_num == 0 || state_num == 5 || state_num == 7) { //IDLE
        // LOOKUP for V1, V2, V3
        // FREEZE values, do nothing
    } else if ((state_num == 1 || state_num == 6 || state_num == 10) && prev_state == 0){ // starting Charge
         // LOOKUP for V1, V2, V3
        temp1 = lookup_c_table(1, V_1, V_2, V_3);
    } else if ((state_num == 3 || state_num == 8 || state_num == 9) && prev_state == 0){ // starting discharge
         // LOOKUP for V1, V2, V3
        temp1 = lookup_d_table(1, V_1, V_2, V_3);
    } else if (state_num == 1 || state_num == 6 || state_num == 10) { // CHARGE
        if (SoC_1 > SoC_HT || SoC_1 < SoC_LT) { // LOOKUP        
            temp1 = lookup_c_table(1, V_1, V_2, V_3); 
        } else { // COULOMB COUNTING
            temp1 = temp1 + charge_1/q1_now*100;
            lookup = 0;
        }
    } else if (state_num == 3 || state_num == 8 || state_num == 9) { // DISCHARGE
        if (SoC_1 > SoC_HT || SoC_1 < SoC_LT) { // LOOKUP
            temp1 = lookup_d_table(1, V_1, V_2, V_3);
        } else { // COULOMB COUNTING
            temp1 = temp1 + charge_1/q1_now*100;
            lookup = 0;
        }
    } else if (state_num == 2) {
        temp1 = 100;
    } else if (state_num == 4) {
        temp1 = 0;
    }

    if (arr_size < 60) { // If Moving average filter is not full yet
      float sum1 = 0;
      if (arr_size > 5) { // Should ignore first 5 values
        SoC_1_arr.push(temp1);
        if (lookup == 1) {
          for (int i = 0; i < arr_size + 1 - 5; i++) {
             sum1 = sum1 + SoC_1_arr[i];
          }
          SoC_1 = sum1/(arr_size + 1 - 5);
        } else { // Do not get moving average filter value for colomb count
          SoC_1 = temp1;
          Serial.println("Not moving average");
        }      
      } else { // Do not push into filter for first 5 values
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

     //Assign previous state
    prev_state = state_num;

    // Now Print all values to serial and SD
    dataString = String(state_num) + "," + String(V_1) + "," + String(SoC_1);
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
    }
    /*
    } else if (cell_num == 2) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 1.0;
            } else if (V_2 >= c_v_2[i] && V_2 < c_v_2[i+1]) {
                return c_SoC[i];
            }
        }
    } else if (cell_num == 3) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 1.0;
            } else if (V_3 >= c_v_3[i] && V_3 < c_v_3[i+1]) {
                return c_SoC[i];
            }
        }
    } 
    */
}

float SMPS::lookup_d_table(int cell_num, float V_1, float V_2, float V_3) {
    //TODO: change to less than equal to operator
    if (cell_num == 1) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 0.0;
            } else if (V_1 <= d_v_1[i] && V_1 > d_v_1[i+1]) {
                Serial.println("Looked up discharge table");
                return d_SoC[i];
            }
        }
    }
    /*
    } else if (cell_num == 2) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 0.0;
            } else if (V_2 <= d_v_2[i] && V_2 > d_v_2[i+1]) {
                return d_SoC[i];
            }
        }
    } else if (cell_num == 3) {
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                return 0.0;
            } else if (V_3 <= d_v_3[i] && V_3 > d_v_3[i+1]) {
                return d_SoC[i];
            }
        }
    }
    */
}
