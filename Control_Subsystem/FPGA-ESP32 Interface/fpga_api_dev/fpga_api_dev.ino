#include "FPGAInterface.h"

#define BASE_ADDRESS 0x40000
#define FPGA_I2C_ADDRESS 0x55

FPGAInterface fpga(&Wire);

ColourObject obj;

void setup() {
  // put your setup code here, to run once:
  fpga.setBusFrequency(400000);
  fpga.setSlaveAddress(FPGA_I2C_ADDRESS);
  fpga.setBaseAddress(BASE_ADDRESS);
  fpga.begin(GPIO_NUM_13, GPIO_NUM_12);

  Serial.begin(115200);
  Serial.setTimeout(10);

}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0;i<5;i++)  {
    obj=fpga.readByIndex(i);
    Serial.print("Colour: ");
    Serial.print(i);
    Serial.print("   Detected: ");
    Serial.print(obj.detected);
    Serial.print("   Angle: ");
    Serial.print(obj.angle);
    Serial.print("   Distance: ");
    Serial.println(obj.distance);
  }
  Serial.println();
  delay(500);

}
