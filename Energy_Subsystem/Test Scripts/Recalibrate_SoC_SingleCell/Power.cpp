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

    if (SD.exists("Diagnose.csv")) {
      SD.remove("Diagnose.csv");
    }
   

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
        Serial.println("Discharge File not open");
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
        Serial.println("Charge File not open");
    }
    myFile.close();

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
        d_SoC_1 = d_SoC_1 - 1/d_size*100;
    }
    myFile.close();

    if (SD.exists(charge_SoC_filename)) {
        SD.remove(charge_SoC_filename);
    }
    myFile = SD.open(charge_SoC_filename, FILE_WRITE);
    for(int i = 0; i < c_iterator; i++){
        if (i == c_iterator - 1) {
            dataString = String(c_v_1[i]) + "," + String(c_v_2[i]) + "," + String(c_v_3[i]) + "," + String(100);
        } else {
            dataString = String(c_v_1[i]) + "," + String(c_v_2[i]) + "," + String(c_v_3[i]) + "," + String(c_SoC_1);      
        }
        // Serial.println(dataString);
        myFile.println(dataString);
        c_SoC[i] = c_SoC_1; // insert value into array
        c_SoC_1 = c_SoC_1 + 1/c_size*100;
    }
    myFile.close();

}

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
    Serial.println("Recorded values");
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
    Serial.println("Cleared lookup table");
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
    Serial.println(dataString);

    if (SD.exists("Stats.csv")) {
        SD.remove("Stats.csv");
    }

    myFile = SD.open("Stats.csv", FILE_WRITE);
    if (myFile){ 
      myFile.println(dataString); 
    } else {
      Serial.println("Stats File not open"); 
    }
    myFile.close();
    Serial.println("Recorded new charge");
}
