This folder contains Sender and Receiver sketches for communication between two Arduino boards by UART.

Sender:

1) Receives a codeword from PC via Serial port
2) Transmits a codeword as a stream of chars by UART via Serial1 port


Receiver:

1) Receives a codeword to Serial1 via UART
2) Decodes the codeword via switch/case statements and transforms it to the desired rover action


Codeword Structure: int0,int1,int2;

int0: indicates the targeted system(for the case of multisystem use)

int1: indicates the desired action(i. e. "Move Forward", "Turn Left")

int2: indicates the value to be passed into action

, is acting as separator

; is acting as terminator



Supposed operation:

Such codeword structure can cover multiple systems and actions, for example:

"Drive subsystem, move forward by 1m"
"Drive subsystem, stop"
"Drive subsystem, turn right by 45 degrees"
