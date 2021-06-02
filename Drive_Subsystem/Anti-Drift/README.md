# Anti-Drift PID
## Description
This folder contains Arduino APIs and code, related to anti-drift PID controller implementation. 

## Operation
1. While moving straight the rover will remember its direction
2. On each loop cycle rover records the current direction it is facing
3. Error is calculated as angle between current and remembered directions
4. This error is passed into proper PID controller that will calculate the control variable
5. The control variable is then saturated and applied as differece in motor speeds to counteract the drifting
6. After the command is executed the error value is reset to zero

## Version Control
- **Ver 1.0**
    * Basic structure of the controller implemented
- **Ver 1.1**
    * PID control works reliable in forward motion
    * Gains are tuned on the flat homogeneous surface, for long range travel

## Objectives
1. Conduct stress tests for the Anti-Drift system.
2. Implement the same PID scheme for backwards motion
