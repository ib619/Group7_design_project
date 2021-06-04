#include "ControlInterface.h"

ControlInterface ci(&Serial);

int command=0;
int speed=0;
int x=0;
int y=0;
unsigned long distance=0;

int soc[3];
int soh[3];
int state=0;
int range=0;

void setup() {
  // put your setup code here, to run once:
  ci.setBaudrate(38400);
  ci.begin();
   delay(2000);
  ci.flushReadBuffer();

}

void loop() {
  // put your main code here, to run repeatedly:
  if(ci.fetchData())  {
    command=ci.getCommand();
    speed=ci.getSpeed();
    x=ci.getPositionX();
    y=ci.getPositionY();
    distance=ci.getTotalDistance();
  }

  for(int i=0;i<3;i++)  {
    ci.writeSOC(i,soc[i]);
    ci.writeSOH(i,soh[i]);
  }
  ci.writeState(state);
  ci.writeRoverRange(range);
  ci.sendUpdates();
}
