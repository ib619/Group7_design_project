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
float d_v[500] = {};
float c_v[500] = {}; 
float d_SoC = 1; 
float c_SoC = 0;

//to generate an average of past 20 values (not moving average)
float v1 = 0;
float v2 = 0;
float v3 = 0;
float v4 = 0;
float v5 = 0;
float v6 = 0;
float v7 = 0;
float v8 = 0;
float v9 = 0;
float v10 = 0;
float v11 = 0;
float v12 = 0;
float v13 = 0;
float v14 = 0;
float v15 = 0;
float v16 = 0;
float v17 = 0;
float v18 = 0;
float v19 = 0;
float v20 = 0;

int arrpointer = 0;       //array pointer
int column = 1;           //column to parse
int columncount = 1;      //start at column 1 not 0
int rowincrement = 10;     //increment the number of parse rows - set to 1 for all rows -> no more than 10,000/20 = 500 rows
int rowcount = 1;         //start at row 1 not 0
String datatemp;          //temp to store datapoint

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
        int filedata = myFile.read();
        Serial.println("Reading discharge file");  
        ////////// START PARSE //////////     
        if((filedata != ',')&&(filedata != ' ')){ //read data vals into string
            datatemp = datatemp += (filedata -'0');
            if (rowcount % rowincrement == 1) {
              if(columncount == column){
                v1 = datatemp.toFloat();
              }
            }
            if (rowcount % rowincrement == 2) {
              if(columncount == column){
                v2 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 3) {
              if(columncount == column){
                v3 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 4) {
              if(columncount == column){
                v4 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 5) {
              if(columncount == column){
                v5 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 6) {
              if(columncount == column){
                v6 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 7) {
              if(columncount == column){
                v7 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 8) {
              if(columncount == column){
                v8 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 9) {
              if(columncount == column){
                v9 = datatemp.toFloat();
              }
            }
            if (rowcount % rowincrement == 10) {
              if(columncount == column){
                v10 = datatemp.toFloat();
              }
            }
            if (rowcount % rowincrement == 11) {
              if(columncount == column){
                v11 = datatemp.toFloat();
              }
            }
            if (rowcount % rowincrement == 12) {
              if(columncount == column){
                v12 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 13) {
              if(columncount == column){
                v13 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 14) {
              if(columncount == column){
                v14 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 15) {
              if(columncount == column){
                v15 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 16) {
              if(columncount == column){
                v16 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 17) {
              if(columncount == column){
                v17 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 18) {
              if(columncount == column){
                v18 = datatemp.toFloat();
              }
            } 
            if (rowcount % rowincrement == 19) {
              if(columncount == column){
                v19 = datatemp.toFloat();
              }
            }       
            if(rowcount % rowincrement == 0){ // to be fair we don't need all values to form a voltage lookup table
                // Column 1 is Voltage, Column 2 is current measured (not neccessary)             
                if(columncount == column){ //push datapoint to array  
                    v20 = datatemp.toFloat();
                    d_v[arrpointer] = (v1+v2+v3+v4+v5+v6+v7+v8+v9+v10+v11+v12+v13+v14+v15+v16+v17+v18+v19+v20)/20;
                    arrpointer++; 
                }
            }        
        }          
        if (filedata == ','){ //end of datapoint
            columncount++;  
        }      
        if (filedata == '\n'){ //end of row         
        //print data temp at end of line
        Serial.println(datatemp.toFloat());
        Serial.println();      
        //reset vars to read next row
        rowcount++; 
        columncount = 1; 
        datatemp = "";         
        ////////// END PARSE //////////    
        }    
    } else {
        Serial.println("File not open");
    }
    myFile.close();

    // Now determine discharge SoC as well and concatenate to new SD
    myFile = SD.open(discharge_SoC_filename, FILE_WRITE);
    if(myFile) {        
        for(int i = 0; i < arrpointer; i++){ //All Subsequent Values
            dataString = String(d_v[i]) + "," + String(d_SoC);
            Serial.println(dataString);
            myFile.println(dataString);
            d_SoC = d_SoC - 1/arrpointer;
            digitalWrite(7, HIGH);
        }
    } else {
        Serial.println("File not open");
    }  

    /////////////////////
    //  CHARGE FILES   //
    /////////////////////
    //Read charge File
    myFile = SD.open(charge_filename);  
    if(myFile) {   
        Serial.println("File open");   
        while (myFile.available()) {     
            int filedata = myFile.read();      
            ////////// START PARSE //////////     
            if((filedata != ',')&&(filedata != ' ')){ //read data vals into string
                datatemp = datatemp += (filedata -'0');
                if (rowcount % rowincrement == 1) {
                  if(columncount == column){
                    v1 = datatemp.toFloat();
                  }
                }
                if (rowcount % rowincrement == 2) {
                  if(columncount == column){
                    v2 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 3) {
                  if(columncount == column){
                    v3 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 4) {
                  if(columncount == column){
                    v4 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 5) {
                  if(columncount == column){
                    v5 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 6) {
                  if(columncount == column){
                    v6 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 7) {
                  if(columncount == column){
                    v7 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 8) {
                  if(columncount == column){
                    v8 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 9) {
                  if(columncount == column){
                    v9 = datatemp.toFloat();
                  }
                }
                if (rowcount % rowincrement == 10) {
                  if(columncount == column){
                    v10 = datatemp.toFloat();
                  }
                }
                if (rowcount % rowincrement == 11) {
                  if(columncount == column){
                    v11 = datatemp.toFloat();
                  }
                }
                if (rowcount % rowincrement == 12) {
                  if(columncount == column){
                    v12 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 13) {
                  if(columncount == column){
                    v13 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 14) {
                  if(columncount == column){
                    v14 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 15) {
                  if(columncount == column){
                    v15 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 16) {
                  if(columncount == column){
                    v16 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 17) {
                  if(columncount == column){
                    v17 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 18) {
                  if(columncount == column){
                    v18 = datatemp.toFloat();
                  }
                } 
                if (rowcount % rowincrement == 19) {
                  if(columncount == column){
                    v19 = datatemp.toFloat();
                  }
                }            
                if(rowcount % rowincrement == 0){ // to be fair we don't need all values to form a voltage lookup table
                    // Column 1 is Voltage, Column 2 is current measured (not neccessary)             
                    if(columncount == column){ //push datapoint to array  
                        v20 = datatemp.toFloat();
                        c_v[arrpointer] = (v1+v2+v3+v4+v5+v6+v7+v8+v9+v10+v11+v12+v13+v14+v15+v16+v17+v18+v19+v20)/20;
                    }
                }        
            }             
            if (filedata == ','){ //end of datapoint
                columncount++;  
            }      
            if (filedata == '\n'){ //end of row         
            //print data temp at end of line
            Serial.print(datatemp.toFloat());
            Serial.println();      
            //reset vars to read next row
            rowcount++; 
            columncount = 1; 
            datatemp = "";
            arrpointer++;  
            }
            ////////// END PARSE //////////    
        }    
    } else {
        Serial.println("File not open");
    }
    myFile.close();

    // Now determine charge SoC as well and concatenate to new SD
    myFile = SD.open(charge_SoC_filename, FILE_WRITE);
    if(myFile) {        
        for(int i = 0; i < arrpointer; i++){ //All Subsequent Values
            dataString = String(c_v[i]) + "," + String(c_SoC);
            Serial.println(dataString);
            myFile.println(dataString);
            c_SoC = c_SoC + 1/arrpointer;
            digitalWrite(8, HIGH);
        }
    } else {
        Serial.println("File not open");
    }   
}


void loop(void) {
  // nothing repeats
}
