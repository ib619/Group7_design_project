
#include "FPGAInterface.h"

#define BASE_ADDRESS 0x40000
#define FPGA_I2C_ADDRESS 0x55
#define LED_BASE 0x1000

FPGAInterface fpga(&Wire);

ColourObject obj;

void setup() {
  // put your setup code here, to run once:
  fpga.setBusFrequency(100000);
  fpga.setSlaveAddress(FPGA_I2C_ADDRESS);
  fpga.setBaseAddress(BASE_ADDRESS);
  fpga.begin(GPIO_NUM_13, GPIO_NUM_12);

  Serial.begin(115200);
  Serial.setTimeout(10);
}

void loop() {
//  for(int i=0;i<10;i++) {
//    fpga.writeLED(i,1);
//    delay(100);
//  }

  for(int i=0;i<10;i++) {
    fpga.toggleLED(i);
    delay(100);
  }

//  for(int i=0;i<10;i++) {
//    fpga.writeLED(9-i,0);
//    delay(100);
//  }

//  for(int i=0;i<5;i++)  {
//    obj=fpga.readByIndex(i);
//    Serial.print("Colour: ");
//    Serial.print(i);
//    Serial.print("   Detected: ");
//    Serial.print(obj.detected);
//    Serial.print("   Angle: ");
//    Serial.print(obj.angle);
//    Serial.print("   Distance: ");
//    Serial.println(obj.distance);
//  }
//  Serial.println();
//  delay(500);

}
