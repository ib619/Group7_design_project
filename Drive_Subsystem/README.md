# Drive Subsystem
## Instructions
1. Install Orientation API
2. Install Drive API
3. Run drive_and_turn.ino

## Arduino Operation
To send the rover into motion type a command to Serial port

### Command structure
#### Distance,Speed,Direction;
1. **Distance:** any positive integer corresponding to desired distance in millimeters
2. **Speed:** integer with range [0,255] corresponding to the motor speed
3. **Direction:** integer with range [-180:180] where:
    - [-180:1] is anticlockwise rotation by specified angle in degrees
    - 0 is forward motion for the earlier specified distance
    - [1:180] is clockwise rotation by specified angle in degrees