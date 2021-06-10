#ifndef POWER_H
#define POWER_H

#include <Arduino.h>
#include <MovingAverage.h>
#include <Wire.h>
#include <INA219_WE.h>
#include <SPI.h>
#include <SD.h>

#define PIN_DISC1 10
#define PIN_DISC2 A7
#define PIN_DISC3 A6

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

        // sends requested state back to the SMPS current controller
        // only get state when instruction is called,
        // otherwise, SMPS will decide its state (determined by default state transitions)
        int get_state();
        void triggerError();
        void reset();
        
        void decode_command(int cmd, int speed, int pos_x, int pos_y, int drive_status, float V_1, float V_2, float V_3);
        float estimate_range(int x0, int y0, int distance, int drive_status) ;
        float estimate_time();

        //TODO: Need to consider balancing for charging as well. Handle in main Arduino file
        void charge(); // 250mA
        void rapid_charge(); // 500mA
        void discharge(); // 500mA
        void rapid_discharge(); // 1A
        void stop(); // stop charge or discharge 

        void determine_discharge_current(int speed, float V_1, float V_2, float V_3);
        float get_discharge_current();
        
        // Recalibrate SOH
        void recalibrate_SOH(); //called by control     
        bool get_recalibrate(); // instruct Arduino to recalibrate.
        void send_current_cap(float q1, float q2, float q3); 
        int get_SOH(int cell_num);
        void clear_lookup();
        void record_curve(int state_num, float V_1, float V_2, float V_3);
        void create_SoC_table();

        // Balancing during charging
        void charge_discharge(float current_measure);
        void charge_balance(float V_1, float V_2, float V_3, float current_measure);
        void discharge_balance(float V_1, float V_2, float V_3, float current_measure);
        float dq1 = 0, dq2 = 0, dq3 = 0; // Stores the amount of charge added/removed within the past 2 minutes. Reset after.
        bool disc1 = 0, disc2 = 0, disc3 = 0; // Account for difference in current when relay is on;
        bool relay_on = 0;

            // Helper function for cv and dv
            // float determine_cv_threshold();
            // float determine_dv_threshold();

        //Compute SoC
        void compute_SOC(int state_num, float V_1, float V_2, float V_3);
        float get_SOC(int cell_num);

        bool cycle_changed = 0;
        void next_cycle();
        int get_cycle(int cell_num);

        // Helper functions called by compute_SOC()
        float lookup_c_table(int cell_num, float V_1, float V_2, float V_3);
        float lookup_d_table(int cell_num, float V_1, float V_2, float V_3);

        bool command_running = 0; 
        // NOTE: command_running even when there is an error
        // to reset, call reset()

        int SD_CS = 10;
        bool error = 0;

    private:
        int state;
        int prev_state = -1;
        bool recalibrating;
        float discharge_current = 0;

        // Need to install Moving Average Library for this
        // Initialise within init method
        MovingAverage<float> SoC_1_arr = MovingAverage<float>(60); // don't go anymore than this
        MovingAverage<float> SoC_2_arr = MovingAverage<float>(60);
        MovingAverage<float> SoC_3_arr = MovingAverage<float>(60);
        int arr_size = 0; // compute manually when FIFO not full

        float current_ref;

        float q1_0 = 1793;
        float q2_0 = 2000.5;
        float q3_0 = 1921.75;

        float q1 = 0, q2 = 0, q3 = 0; // coulomb counting for battery (to determine cycles)
        float cycle1 = 0, cycle2 = 0, cycle3 = 0; // number of cycles, imported from SD

        float q1_now, q2_now, q3_now; // total capacity, determined during recalibration
        float SoH_1, SoH_2, SoH_3;
        float SoC_1, SoC_2, SoC_3;

        // TODO: load these values from initialisation files
        float d_ocv_l = 3150;
        float d_ocv_u = 3300;
        float c_ocv_u = 3450;
        float c_ocv_l = 3300;

        //FIXME: Instead of using voltage threshold, use an SoC Threshold
        float SoC_LT = 20;
        float SoC_HT = 80;

        String discharge_SoC_filename = "dv_SoC.csv";
        String charge_SoC_filename = "cv_SoC.csv";
        String capacity_filename = "Stats.csv";
        String cycle_filename = "Cycles.csv";

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

        float drive_speed[18] = {};
        float drive_power[18] = {};

        int c_iterator = 0;
        int d_iterator = 0;

        // Range Estimation
        float x1 = 0, y1 = 0;
        float SoC_1_start=100, SoC_2_start=100, SoC_3_start=100;
};

#endif
