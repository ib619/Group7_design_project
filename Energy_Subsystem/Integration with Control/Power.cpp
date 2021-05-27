#include "Power.h"

/*

Flow chart (typical)
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

*/

//TODO: record SoC periodically

SMPS::SMPS() {};

// TODO: Also need to load in threshold values
void SMPS::init(Stats initial) {

    /* Need to Initialise
        Current statistics: current maximal charge, current SOC, current SOH
        Charge discharge curves, from SD
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

    int columncount = 1; 
    File myFile;
    String content;

    // Load values from last run
    // Just 1 row
    // 11 items: q1_now, q2_now, q3_now, SOH_1, SOH_2, SOH_3, gross_SOH, SoC_1, SoC_2, SoC_3, gross_SOC  
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
                if (i == 3) {
                    SoH_1 = content;
                }
                if(i == 4) {
                    SoH_2 = content;
                }
                if (i == 5) {
                    SoH_3 = content;
                }
                if (i == 6) {
                    gross_SoH = content;
                }
                if (i == 7) {
                    SoC_1 = content;
                }
                if (i == 8) {
                    SoC_2 = content;
                }
                if (i == 9) {
                    SoC_3 = content;
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
                if (content == "") {
                    break;
                    Serial.println("Insertion Complete");    
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
                if (content == "") {
                    break;
                    Serial.println("Insertion Complete");    
                }                 
            }
        }
    } else {
        Serial.println("File not open");
    }
    myFile.close();
}

Stats SMPS::get_Stats() {
    Stats output = {
        .q1_now = q1_now,
        .q2_now = q2_now,
        .q3_now = q3_now,

        .SoH_1 = SoH_1,
        .SoH_2 = SoH_2,
        .SoH_3 = SoH_3,
        .gross_SoH = gross_SoH,

        .SoC_1 = SoC_1,
        .SoC_2 = SoC_2,
        .SoC_3 = SoC_3,
        .gross_SoC = gross_SoC
    };
    return output;
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

void SMPS::decode_command(int cmd) {
    //TODO:
}

float SMPS::estimate_range(float disTravelled, float SoC_drop) {
    // Using total distance travelled, and the drop in SoC since start
    //Assuming a linear relationship between SoC_drop and dist travelled
    float dist_per_drop = disTravelled/SoC_drop;
    return dist_per_drop*gross_SoC;
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

    SoH_1 = q1_now/q1_0;
    SoH_2 = q2_now/q2_0;
    SoH_3 = q3_now/q3_0;
    gross_SoH = (SoH_1+ SoH_2+SoH_3)/3;
}

void SMPS::compute_SOC(int state_num, float V_1, float V_2, float V_3, float charge_1, float charge_2, float charge_3) {
    //TODO: identify first value and freeze instead of lookup?
    //TODO: test for edge cases?

    float temp1 = SoC_1;
    float temp2 = SoC_2;
    float temp3 = SoC_3;

    if (state_num == 0) { //IDLE
        // LOOKUP for V1, V2, V3
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                temp1 = 0;
                break;
            } else if (V_2 < d_v_2[i] && V_2 > d_v_2[i+1]) {
                temp2 = d_SoC[i];
                break;
            }
        }
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                temp2 = 1;
                break;
            } else if (V_2 > c_v_2[i] && V_2 < c_v_2[i+1]) {
                temp2 = c_SoC[i];
                break;
            }
        }
        for (int i=0; i < 100; i++) {
            if (i == 99) {
                temp3 = 1;
                break;
            } else if (V_3 > c_v_3[i] && V_3 < c_v_3[i+1]) {
                temp3 = c_SoC[i];
                break;
            }
        }
    } else if (state_num == 1 || state_num == 6 || state_num == 10) { // CHARGE
        if (V_1 > c_ocv_u_1 || V_1 < c_ocv_l_1) { // LOOKUP        
            for (int i=0; i < 100; i++) {
                if (i == 99) {
                    temp1 = 1;
                    break;
                } else if (V_1 > c_v_1[i] && V_1 < c_v_1[i+1]) {
                    temp1 = c_SoC[i];
                    break;
                }
            }
            
        } else { // COULOMB COUNTING
            temp1 = temp1 + charge_1/q1_now;
        }
        if (V_2 > c_ocv_u_2 || V_2 < c_ocv_l_2) { // LOOKUP
            for (int i=0; i < 100; i++) {
                if (i == 99) {
                    temp2 = 1;
                    break;
                } else if (V_2 > c_v_2[i] && V_2 < c_v_2[i+1]) {
                    temp2 = c_SoC[i];
                    break;
                }
            }
            
        } else { // COULOMB COUNTING  
            temp2 = temp2 + charge_2/q2_now;
        }
        if (V_3 > c_ocv_u_3 || V_1 < c_ocv_l_3) { // LOOKUP  
            for (int i=0; i < 100; i++) {
                if (i == 99) {
                    temp3 = 1;
                    break;
                } else if (V_3 > c_v_3[i] && V_3 < c_v_3[i+1]) {
                    temp3 = c_SoC[i];
                    break;
                }
            }          
        } else { // COULOMB COUNTING
            temp3 = temp3 + charge_3/q3_now;
        }
    } else if (state_num == 3 || state_num == 8 || state_num == 9) { // DISCHARGE
        if (V_1 > d_ocv_u_1 || V_1 < d_ocv_l_1) { // LOOKUP
            for (int i=0; i < 100; i++) {
                if (i == 99) {
                    temp1 = 0;
                    break;
                } else if (V_1 < d_v_1[i] && V_1 > d_v_1[i+1]) {
                    temp1 = d_SoC[i];
                    break;
                }
            }
        } else { // COULOMB COUNTING
            temp1 = temp1 + charge_1/q1_now;
        }
        if (V_2 > d_ocv_u_2 || V_2 < d_ocv_l_2) {
            // LOOKUP
            for (int i=0; i < 100; i++) {
                if (i == 99) {
                    temp2 = 0;
                    break;
                } else if (V_2 < d_v_2[i] && V_2 > d_v_2[i+1]) {
                    temp2 = d_SoC[i];
                    break;
                }
            }
        } else { // COULOMB COUNTING
            temp2 = temp2 + charge_2/q2_now;
        }
        if (V_3 > d_ocv_u_3 || V_1 < d_ocv_l_3) { // LOOKUP
            for (int i=0; i < 100; i++) {
                if (i == 99) {
                    temp3 = 0;
                    break;
                } else if (V_3 < d_v_3[i] && V_3 > d_v_3[i+1]) {
                    temp3 = d_SoC[i];
                    break;
                }
            }
        } else { // COULOMB COUNTING
            temp3 = temp3 + charge_3/q3_now;
        }
    } else if (state_num == 5 || state_num == 7) {
        // FREEZE VALUES, do nothing
    } else if (state_num == 2 || state_num == 4) {
        temp1 = 1;
        temp2 = 1;
        temp3 = 1;
    }

    // Moving average
    float sum1 = 0, sum2 = 0, sum3 = 0;
    // If Moving average filter is not full yet
    if (arr_size < 10) {
        SoC_1_arr.push(temp1);
        SoC_2_arr.push(temp2);
        SoC_3_arr.push(temp3);
        for (int i = 0; i < arr_size + 1; i++) {
            sum1 = sum1 + SoC_1_arr[i];
            sum2 = sum2 + SoC_2_arr[i];
            sum3 = sum3 + SoC_3_arr[i];
        }
        arr_size = arr_size + 1;
        SoC_1 = sum1/arr_size;
        SoC_2 = sum2/arr_size;
        SoC_3 = sum3/arr_size;
    } else {
        SoC_1 = SoC_1_arr.push(temp1).get();
        SoC_2 = SoC_2_arr.push(temp2).get();
        SoC_3 = SoC_3_arr.push(temp3).get();
    }

     //CALCULATE GROSS SOC
    gross_SoC = (SoC_1+ SoC_2+SoC_3)/3;
    prev_state = state_num;

    // Now Print all values to serial and SD
    dataString = String(state_num) + "," + String(V_1) + "," + String(V_2) + "," + String(V_3) + "," + String(SoC_1) + "," + String(SoC_2)  + "," + String(SoC_3  + "," + String(q1_now) + "," + String(q2_now)  + "," + String(q3_now);
    Serial.println(dataString);

    File dataFile = SD.open("Series_Stats.csv", FILE_WRITE);
    if (dataFile){ 
      dataFile.println(dataString);
    } else {
      Serial.println("File not open"); 
    }
    dataFile.close();
}

void SMPS::record_curve(int state_num, float V_1, float V_2, float V_3) {
    //TODO: decide thresholds after
    File myFile;
    String dataString;

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
    float d_SoC_1 = 1, d_SoC_2 = 1, d_SoC_3 = 1;
    float c_SoC_1 = 0, c_SoC_2 = 0, c_SoC_3 = 0;

    float d_size = static_cast<float>(d_iterator);
    float c_size = static_cast<float>(c_iterator);

    File myFile;
    String dataString;

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
        Serial.println(dataString);
        myFile.println(dataString);
        d_SoC = d_SoC - 1/d_size;
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
        Serial.println(dataString);
        myFile.println(dataString);
        c_SoC = c_SoC + 1/c_size;
    }
    myFile.close();

    int iterator = 0;

    //TODO: immediately store this into the SoC tables

    //TODO: Determine thresholds: d_ocv_u, d_ocv_l, c_ocv_u, c_ocv_l
    for (int i = 0; i < d_iterator-1; i++) {
        if (d_v_1[i+1] - d_v_1[i] > -0.12 * 120 && V_1 > 3200) {
            d_ocv_u_1 = d_v_1[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < d_iterator-1; i++) {
        if (d_v_1[i+1] - d_v_1[i] < - 0.08 * 120 && V_1 < 3100) {
            d_ocv_u_1 = d_v_1[i];          
            break;
        }
    }
    
    iterator = 0;
    for (int i = 0; i < d_iterator-1; i++) {
        if (d_v_2[i+1] - d_v_2[i] > -0.12 * 120 && V_2 > 3200) {
            d_ocv_u_2 = d_v_2[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < d_iterator-1; i++) {
        if (d_v_2[i+1] - d_v_2[i] < - 0.08 * 120 && V_2 < 3100) {
            d_ocv_u_2 = d_v_2[i];          
            break;
        }
    }

    iterator = 0;
    for (int i = 0; i < d_iterator-1) {
        if (d_v_3[i+1] - d_v_3[i] > -0.12 * 120 && V_3 > 3200) {
            d_ocv_u_3 = d_v_3[i];
            break;
        }
    }
    for (int i = iterator; i < d_iterator-1; i++) {
        if (d_v_3[i+1] - d_v_3[i] < - 0.08 * 120 && V_3 < 3100) {
            d_ocv_u_3 = d_v_3[i];          
            break;
        }
    }

    iterator = 0;
    for (int i = 0; i < c_iterator-1; i++) {
        if (c_v_1[i+1] - c_v_1[i] < 0.12 * 120 && V_1 < 3300) {
            c_ocv_l_1 = c_v_1[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < c_iterator-1; i++) {
        if (c_v_1[i+1] - c_v_1[i] > 0.1 * 120 && V_1 > 3400) {
            c_ocv_u_1 = c_v_1[i];          
            break;
        }
    }

    iterator = 0;
    for (int i = 0; i < c_iterator-1; i++) {
        if (c_v_2[i+1] - c_v_2[i] < 0.12 * 120 && V_2 < 3300) {
            c_ocv_l_2 = c_v_2[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < c_iterator-1; i++) {
        if (c_v_2[i+1] - c_v_2[i] > 0.1 * 120 && V_2 > 3400) {
            c_ocv_u_2 = c_v_2[i];          
            break;
        }
    }

    iterator = 0;
    for (int i = 0; i < c_iterator-1; i++) {
        if (c_v_3[i+1] - c_v_3[i] < 0.12 * 120 && V_3 < 3300) {
            c_ocv_l_3 = c_v_3[i];
            iterator = i;
            break;
        }
    }
    for (int i = iterator; i < c_iterator-1; i++) {
        if (c_v_3[i+1] - c_v_3[i] > 0.1 * 120 && V_3 > 3400) {
            c_ocv_u_3 = c_v_3[i];          
            break;
        }
    }
}
