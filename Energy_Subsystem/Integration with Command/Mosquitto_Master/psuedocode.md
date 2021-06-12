# Psuedocode for Master File

## Common Abbreviations

| Abbreviation | Word |
| -- | --|
| cmd | command |
| SoC | State of Charge |
| SoH | State of Health |


## 1 Initialisation and Setup

```
Define pins
Initialise counters
    int_count (200Hz): triggered by TIMERA0's loop_trigger. reset every second.
    sec_count (1 Hz): reset every 3 minutes (voltage data collected every 3 mins during recalibration)
    rly_timer (200Hz): reset every 5 second (relays on battery boards triggered every 5 seconds)
    rest_timer (1 Hz): ensure that battery stays in CHARGE_REST/ DISCHARGE_REST for 30 secs.
    rapid_timer (1 Hz): ensure that battery does not stay in RAPID_CHARGE/ RAPID_DISCHARGE for more than 10 secs.
Initialise INA219 (current library sensor)
Initialise SMPS (library for processing SoC and SoH data)
Get SoH and cycle number - saved in SD card, loaded upon running mySMPS.init()

Print to serial: SOH_1, SOH_2, SOH_3, cycle1, cycle2, cycle3
```

## 2 Main Loop

``` C++

if (command received) {
    Decode command
    if (not in error and not in recalibration) {
        state = mySMPS.get_state()
        if (cmd == recalibrate) {
            Clear lookup tables (reset to zero.)
        }
    } else if (cmd == reset) {
        Reset to IDLE (reset can always be run, even during recalibration)
    } else {
        Ignore command sent during recalibration
    }
}

(every 5ms) {
    if (cell under-voltage or over-voltage) {
        state = ERROR
        Identify type of error for cell (no error, over-voltage, under-voltage)
    }

    Obtain current measurement

    if (charging) {
        Do not use PID controllers.
    } else if (cosntant voltage charging) {
        Use both outer loop voltage then inner loop current PID controller.
    } else {
        Use current PID controller.
    }
    Saturate PWM values within bound and write to SMPS

    int_count++, rly_timer++
}

(100 < rly_timer < 126) {
    Sequentially - for each cell switch on relay, measure voltage, then switch off relay
    (reset relay timer every 5 seconds)
}

// State Machine
(every sec) {
    switch (state) {
        case IDLE: {  // 0
            current_ref =  Get discharge current required during IDLE.
        }
        case CHARGE: { // 1
            Run MPPT Algorithm to select PWM.
            Balance Cells

            if (full: any one battery reached upper voltage limit) {
                state = CV_CHARGE
            }
        }
        case CHARGE_REST: { // 2
            if (not recalibrating) {
                state = IDLE
            }
            if (recalibrating) {
                Wait for 30 seconds.
                if (already fully discharged and charged) {
                    state = RECAL_Done // Recalibration is complete
                } else {
                    state = SLOW_DISCHARGE
                }
            }
        }
        case SLOW_DISCHARGE: { // 3
            current_ref = -250;
            Balance Cells
            if (empty: any one cell reached lower voltage limit) {
                state = DISCHARGE_REST
            }
        }
        case DISCHARGE_REST: { // 4
            if (not recalibrating) {
                state = IDLE
            }
            if (recalibrating) {
                Notify: discharge is completed.
                Wait for 30 seconds.
                Then send current capacity (counted using coulomb counting to SD file)
                state = CHARGE
            }
        }
        case ERROR: { // 5
            Set current_ref to 0 and do nothing.
        }
        case CV_CHARGE: { // 6
            vref = 3600
            if (current measure reaches 0) {
                state = CHARGE_REST
            }
        }
        case RECAL_DONE { // 7
            Notify: discharge completed
            Rebuild SoC lookup table (need to halt interrupts for this)

            Print to serial: SOH_1, SOH_2, SOH_3, cycle1, cycle2, cycle3
        }
        case DISCHARGE: { // 8
            current_ref =  Get discharge current required, depending on speed of drive
            Balance cells
            if (empty: any one cell reached lower voltage limit) {
                state = DISCHARGE_REST
            }
        }     
    }

    if (not recalibrating) {
        get SoC values
    }

    Print diagnostic values: state, V_1,2,3, current_ref,measure, SoC_1,2,3, discharge1,2,3 (for balancing)

    Estimate remaining range
    Estimate remaining runtime
    Get number of cycles if changed

    Print to serial: state, SoC_1,2,3, range, remaining runtime, error1,2,3
}

(every 3 min) {
    if (recalibrating) {
        Record V_1, V_2, V_3
    }
}

```

## 3 Main functions within the SMPS class

``` C++
void init(); 
```
Grabs to current capacity in terms of charge, charge and discharge curves, number of cycles from SD

``` C++
void decode_command(int cmd, int speed, int pos_x, int pos_y, int drive_status, float V_1, float V_2, float V_3);
```
Decodes the command. If rover is at charging hub (0,0),  then it sends a command to charge. By default, it tells the SMPS to stay in idle.  Whenever the drive status is running, it instructs the SMPS to discharge at an appropriate current.

``` C++
int estimate_range(int x0, int y0, float distance, int drive_status);
```
Algorithm does not return a value when the SoC drop is less than 20%. Beyond that the range is highly inaccurate and hence infinite for practical purposes. 
Assumes that the drop in range is linear to drop in SoC (which is clearly not true given that the motor's idle power is significant, and the ESP32 and DE10 draw a significant amount of power too. Therefore, remaining run time provides a better image of the remaining range of the rover)

``` C++
int estimate_time(float V_1, float V_2, float V_3);
```
Evaluates the remaining run time, by comparing the watt-hour capacity to the power drawn. The assumption is that the power drawn stays at approximately 2W (which is the IDLE power of the rover).

```C++
void determine_discharge_current(int speed, float V_1, float V_2, float V_3);
```
Having knowledge of the drive speed, it determines the discharge current, by looking up a speed-power table. The power value accounts for the power drawn from the ESP32 and DE10 as well (the power for the drive and energy arduino are accounted within the SMPS inefficiencies already). The current is then evaluated with I = Power/(V_1+V_2+V_3), and scaled by 1/0.9 to account for the SMPS inefficiencies.

```C++
void send_current_cap();
```
At the end of a discharge during recalibration, the class records the current capacity in terms of charge, obtained by coulomb counting during the discharge. The SoH values are evaluated as well and recorded on an SD.

```C++
void create_SoC_table();
```
At the end of recalibration, the class identifies the entries corresponding to a full discharge and a full charge. Since the intention is to make the SoC linear against the charge time (length of the full discharge and full charge voltage list), the SoC values are:
    - Mapped to 0-100% for the full charge voltage list
    - Mapped to 100% to 0% for the full discharge voltage list
These mappings are effectively the lookup tables used in the future. The assumption here is that the cells are perfectly balanced during recalibration.

```C++
void charge_discharge(float current_measure);
void charge_balance(float V_1, float V_2, float V_3, float current_measure);
void discharge_balance(float V_1, float V_2, float V_3, float current_measure);
```
The rationale of balancing is to decrease the current through higher SoC cells during charging, and through lower SoC cells during discharging. This method turns on and off the discharge pin based on this rationale. It also accounts for the difference in current through the cells when the discharge circuit is connected, to provide a more accurate coulomb counting value.
If the relays are on during this period, the reduction in charge flowing is accounted for as well.

```C++
void compute_SOC(int state_num, float V_1, float V_2, float V_3);
```
This is the most important function. In general, an upper and lower voltage threshold is defined for discharging and charging. The rationale is:
- Below and above these thresholds, the SoC is obtained by looking up the charge/ discharge table. A moving average filter of size 120 is also applied.
- Within these thresholds, the SoC is obtained by coulomb counting. Moving average is not adopted here, since the difference in measurements between time instances are too small.


```C++
void next_cycle();
```
The class keep tracks of the coulomb count of each cell. Upon reaching the cell charge capacity, the cycle value is incremented by 0.5 (a full cycle involves a full charge and full discharge).
