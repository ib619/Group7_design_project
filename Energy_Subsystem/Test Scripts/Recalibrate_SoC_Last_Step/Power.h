#ifndef POWER_H
#define POWER_H

#include <Arduino.h>
#include <MovingAverage.h>
#include <Wire.h>
#include <INA219_WE.h>
#include <SPI.h>
#include <SD.h>

struct Stats {
    float q1_now;
    float q2_now;
    float q3_now;

    float SoC_1;
    float SoC_2;
    float SoC_3;
};

class SMPS {
    public:
        SMPS();
        void init(); // grab from SD card, sensors

        void create_SoC_table();
        // Helper function for cv and dv
        // float determine_cv_threshold();
        // float determine_dv_threshold();
        
        bool command_running = 0; 
        // NOTE: command_running even when there is an error
        // to reset, call reset()

        int SD_CS = 10;
    
    private:
        int state;
        int prev_state = -1;
        bool recalibrating;
        bool error;

        // Need to install Moving Average Library for this
        // Initialise within init method
        MovingAverage<float> SoC_1_arr = MovingAverage<float>(60);
        MovingAverage<float> SoC_2_arr = MovingAverage<float>(60);
        MovingAverage<float> SoC_3_arr = MovingAverage<float>(60);
        int arr_size = 0; // compute manually when FIFO not full

        float current_ref;

        float q1_0 = 1793;
        float q2_0 = 2000.5;
        float q3_0 = 1921.75;

        float q1_now, q2_now, q3_now;
        // float SoH_1, SoH_2, SoH_3;
        float SoC_1, SoC_2, SoC_3;

        // TODO: load these values from initialisation files
        // These values are decided after reading the entire discharge
        // or charge cycle (post recalibration, deterministic)
        // FIXME: Instead of using voltage threshold use SoC threshold
        float d_ocv_l = 3100;
        float d_ocv_u = 3300;
        float c_ocv_u = 3450;
        float c_ocv_l = 3300;
        float SoC_LT = 20;
        float SoC_HT = 80;

        String discharge_SoC_filename = "dv_SoC.csv";
        String charge_SoC_filename = "cv_SoC.csv";
        String dataString;
        File myFile;

        float d_v_1[100] = {};
        float c_v_1[100] = {};
        float d_v_2[100] = {};
        float c_v_2[100] = {};
        float d_v_3[100] = {};
        float c_v_3[100] = {};   

        float d_SoC[100] = {};
        float c_SoC[100] = {};

        int c_iterator = 100;
        int d_iterator = 100;
};

#endif
