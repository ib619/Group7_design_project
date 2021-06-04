#include <CSV_Parser.h>
#include <SPI.h>
#include <SD.h>

// Which cell are you characterising?
int CELL = 1;

// SD
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 10;
int row = 0;
char d;
int i;

// Files containing the charge and discharge curves (voltage and current)
File chargeFile;
File dischargeFile;

// Placeholder for csv file
char * csv_str;

// File names (Placeholder)
String discharge_filename = "DischargeSoC1.csv";
String charge_filename = "ChargeSoC1.csv";
String SoH_filename = "SoHStats1.csv";

// Operation completed?
boolean done;


//By fat16lib
size_t readField(File* file, char* str, size_t size, char* delim) {
  char ch;
  size_t n = 0;
  while ((n + 1) < size && file->read(&ch, 1) == 1) {
    // Delete CR.
    if (ch == '\r') {
      continue;
    }
    str[n++] = ch;
    if (strchr(delim, ch)) {
        break;
    }
  }
  str[n] = '\0';
  return n;
}

void setup() {

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
    discharge_filename = "DischargeSoC1.csv";
    charge_filename = "ChargeSoC1.csv";
    SoH_filename = "SoHStats1.csv";
  } else if (CELL == 2) {
    discharge_filename = "DischargeSoC2.csv";
    charge_filename = "ChargeSoC2.csv";
    SoH_filename = "SoHStats2.csv";
  }  else if (CELL == 3) {
    discharge_filename = "DischargeSoC2.csv";
    charge_filename = "ChargeSoC2.csv";
    SoH_filename = "SoHStats3.csv";
  } else {
    Serial.println("\nWe only have 3 cells...");
  }
  
  // Obtain discharge csv
  dischargeFile = SD.open(discharge_filename, FILE_WRITE);
  if (dischargeFile){
    Serial.println("File open");
     // Check the number of rows in the file
     while(dischargeFile.available()) {
      d = dischargeFile,read(); 
      if (d == '\n') {
        row = row + 1;     
      }
    }    
  } else {
    Serial.println("File not open");
  }
  dischargeFile.close();

  
}

void loop() {
  // nothing happens repeatedly;
}
