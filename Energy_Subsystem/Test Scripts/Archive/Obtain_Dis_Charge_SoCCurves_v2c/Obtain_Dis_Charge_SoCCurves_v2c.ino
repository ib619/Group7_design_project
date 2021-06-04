//libs
#include <SPI.h>
#include <SD.h>
#include <String.h>
//#include <LinkedList.h>

/*
Aim:
    From the discharge and charge curves, obtain the SoC lookup table
    And save it in the files "dvSoc1.csv" "cvSoc1.csv"

Intended flow chart:
    1. Access discharge voltage table and save in an array
    2. Using the discharge voltage table and the array size, save it in a new file alongside the SoC
    3. Repeat for charge voltage table

WARNING
  Sketch uses 20331 bytes (41%) of program storage space. Maximum is 49152 bytes.
  Global variables use 4945 bytes (80%) of dynamic memory, leaving 1199 bytes for local variables. Maximum is 6144 bytes.
  Low memory available, stability problems may occur.
  I STILL CANT INSTALL LINKEDLIST!!

  Although arguably linkedlist might not be the best solution since we need to access values in the middle of the array

*/

// Which cell are you characterising?
int CELL = 1;

// SD
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 10;

//SD vars
File myFile;

//array of parsed data
float d_v[200] = {};
float c_v[200] = {}; 
float d_SoC = 1.00; 
float c_SoC = 0.00;

String line = "";

int arrpointer = 0;       //array pointer

// File names (Placeholder)
String discharge_filename = "dv1.csv";
String charge_filename = "cv1.csv";
String discharge_SoC_filename = "dvSoc1.csv";
String charge_SoC_filename = "cvSoc1.csv";

String dataString;

void setup(){

    Serial.begin(9600);

    //Check for the SD Card
    Serial.println("\nInitializing SD card...");
    if (!SD.begin(chipSelect)) {
    Serial.println("* is a card inserted?");
    while (true) {} //It will stick here FOREVER if no SD is in on boot
    } else {
    Serial.println("Wiring is correct and a card is present.");
    }

    // Batcurves stores the discharge and charge curves of the battery and compares it to the SoC
    if (CELL == 1) {
        discharge_filename = "dv1.csv";
        charge_filename = "cv1.csv";
        discharge_SoC_filename = "dvSoc1.csv";
        charge_SoC_filename = "cvSoc1.csv";
    } else if (CELL == 2) {
        discharge_filename = "dv2.csv";
        charge_filename = "cv2.csv";
        discharge_SoC_filename = "dvSoC2.csv";
        charge_SoC_filename = "cvSoC2.csv";
    }  else if (CELL == 3) {
        discharge_filename = "dv3.csv";
        charge_filename = "cv3.csv";
        discharge_SoC_filename = "dvSoC3.csv";
        charge_SoC_filename = "cvSoC3.csv";
    } else {
        Serial.println("\nWe only have 3 cells...");
    }

    /////////////////////
    // DISCHARGE FILES //
    /////////////////////
    //Read Discharge File
    myFile = SD.open(discharge_filename);  
    if(myFile) {   
        Serial.println("Discharge file open");            
        while (myFile.available() != 0 && arrpointer < 200) {
          Serial.println("Reading line");  
          line = myFile.readStringUntil('\n');
          Serial.println(line);
          Serial.println();
          d_v[arrpointer] = line.toFloat();
          arrpointer++;
          if (line == "") //no blank lines are anticipated           
            break; 
        }    
    } else {
        Serial.println("File not open");
    }
    myFile.close();

    float arrpt_flt = static_cast<float>(arrpointer);

    // Now determine discharge SoC as well and concatenate to new SD
    myFile = SD.open(discharge_SoC_filename, FILE_WRITE);
    if(myFile) {        
        for(int i = 0; i < arrpointer; i++){ //All Subsequent Values
            dataString = String(d_v[i]) + "," + String(d_SoC);
            Serial.println(dataString);
            myFile.println(dataString);
            d_SoC = d_SoC - 1/arrpt_flt;
            digitalWrite(7, HIGH);
        }
    } else {
        Serial.println("File not open");
    }
    Serial.println("Array pointer = " + String(arrpointer));
    arrpointer = 0;
   
}


void loop(void) {
  // nothing repeats
}
