# Drive Subsystem
## Description
This folder contains the Orientation API and Drive API for the rover(in DriveControl folder). Additionally it contains arduino sketches Drive_Control_integration.ino, ESP32_Command_Simulation.ino for drive system testing.
The purpose of this testing is to ensure the Drive Control Arduino is able to receive and execute commands sent by ESP32.

* ESP32_Command_Simulation is able to receive commands from user via Serial port, and sending these commands to Drive System via Serial1 (UART).
* Drive_Control_integration is able to receive drive commands from another Arduino/ESP32 via Serial1 (UART), decode them, and perform the desired actions.

## Instructions
Setup your Rover as shown in test_setup.png

### Drive_Control_integration.ino
1. Install Drive API
2. Install Orientation API
3. Install ControlInterface API (from Control_Subsystem folder)
4. Upload the code to Drive Control Arduino

### ESP32_Command_Simulation.ino
1. Install DriveInterface API (from Control_Subsystem folder)
2. Run the code on ESP32 simulation Arduino
3. Enter a command into the Serial port

#### Command structure
##### DriveMode,Distance,Speed,Direction
1. **DriveMode:** 0 for emergency stop, 1 for discrete driving mode
2. **Distance:** any integer corresponding to desired distance in millimeters:
    - for positive distance Rover moves forward
    - for negative distance Rover moves backwards
    - BUG: distance of 0 causes an error, therefore must never be set so 
3. **Speed:** integer with range [0,255] corresponding to the motor speed
4. **Direction:** integer with range [-180:180] where:
    - [-180:1] is anticlockwise rotation by specified angle in degrees
    - 0 is straight motion for the earlier specified distance
    - [1:180] is clockwise rotation by specified angle in degrees

**IMPORTANT:** When direction command is non-zero, the rover will turn first, and then move straight for specified distance. In order to only turn without moving afterwards, the distance must be set to 1.

## Further Objectives
1. Design an Anti-Drift algorithm, for preventing side drifting when moving straight
2. Attempt to fix the bug causing 0 distance leading to an error
