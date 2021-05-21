# Energy HQ

I have currently separated the development of Batteries and PV Panels into 2 different folders. Check out the relevant research in the different folders.

## Updated Roadmap

### Aim for demo

We do not need to implement the hardware for the batteries and PV charging since the hardware isn't there for integration. For the video demo, most code for the energy module can be omitted.

### Aim for codebase

For the control/command side, this means the Arduino Nano Every for drive and energy will need to receive:

- Anything required by the drive submodule
- Distance travelled (from drive)
- Instructions to:
  - Estimate the state of health SoH of battery (which discharges and charges the entire battery, taking around 4 hours in total)
    - Might need another instruction to obtain the SoC curve, since this operation doesn't fit into the slow/fast loop structure of my code (see `Battery_Charge_Cycle_Logged_V1.1`). Basically numerically assessing the SoC takes a lot of time, which would likely break the interrupt mechanism in place.
  - Charge the battery
    - Which also involves switching on the solar panels
    - And to charge the battery with rapid charging, though this degrades the battery much quicker in practical terms
  - Discharge the battery

Similarly, the arduino will send:

- Range estimation information
- State of charge information
  - And also time till completion
- Status of battery: 
  - The battery charges with respect to a 6-state state machine structure, namely:
    - 0 IDLE
    - 1 CONSTANT CURRENT CHARGE (yellow LED)
    - 2 CHARGE REST
    - 3 DISCHARGE
    - 4 DISCHARGE REST
    - 5 ERROR (red LED) (must go to 0 next and restart)
    - 6 CONSTANT VOLTAGE CHARGE (blinking yellow LED)
    - 7 CHARGE COMPLETE
  - The status of the battery is important. For example, in a real setup, when the battery is in an error state, it will literally stop delivering power to other modules and the motor. Most of the time however, it can simply be resolved by resetting the state machine to 0, and then entering charge or discharge mode.
  - We might make decisions depending on the last state of the battery. The states experienced by the batteries are as follows:
    - Full discharge: 3
    - Full charge: 1 > 6
    - SoH recalibration: 0 > 1 > 2 > 3 > 4 > 1 > 2 > 6


## Materials

- Solar panels
- SMPS
- Arduino
- AA LiFePO4

## Main Tasks

The energy subsystem will provide the rover with charged batteries using solar panels. The main tasks in this subsystem are:

1. Charge batteries.
2. Battery charge profile design.
3. Battery charge status estimation. Time till completion?
4. Battery balancing algorithm.
5. PV MMPT algorithm.
6. System Integration and Test.
7. Rover range estimation.
8. Prevent explosion/melt.

## Main circuits to build/ deal with

1. USB - Battery
2. Battery - Resistor (Current Aim)
3. PV - Battery (Ultimate aim)
4. PV - resistor (Current Aim)