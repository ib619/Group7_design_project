#include "FPGAInterface.h"

FPGAInterface::FPGAInterface(TwoWire *tw)   {
    i2c=tw;
}

void FPGAInterface::begin() {
    i2c->begin();
    i2c->setClock(bus_frequency);
}

void FPGAInterface::begin(gpio_num_t sda, gpio_num_t scl)   {
    i2c->begin(sda,scl);
    i2c->setClock(bus_frequency);
}

void FPGAInterface::setBusFrequency(long freq)  {
    bus_frequency=freq;
}

void FPGAInterface::setSlaveAddress(int saddr)  {
    slave_address=saddr;
}

void FPGAInterface::setBaseAddress(long ba) {
    base_address=ba;
}

ColourObject FPGAInterface::readByIndex(int index)  {
    ColourObject tmp;
    char d_high=0;
    char d_low=0;
    _setAddress((4*index) + base_address);
    i2c->requestFrom(slave_address,4);
    while(i2c->available()) {
        tmp.detected=i2c->read();
        tmp.angle=(int8_t)i2c->read();
        d_low=i2c->read();
        d_high=i2c->read();
    }
    tmp.distance=(d_high<<8)+d_low;
    return tmp;
}

void FPGAInterface::writeLED(int n, int value) {
    if(value!=0) {
        led |= int(pow(2,n));
    }
    else    {
        led &= ~int(pow(2,n));
    }
    _setLEDRegister();
}

void FPGAInterface::toggleLED(int n)    {
    led ^= int(pow(2,n));
    _setLEDRegister();
}

void FPGAInterface::offLEDs()    {
    led=0;
    _setLEDRegister();
}

void FPGAInterface::onLEDs()  {
    led=1023;
    _setLEDRegister();
}

void FPGAInterface::_setAddress(long addr)  {
    i2c->beginTransmission(slave_address);
    i2c->write(addr>>24);
    i2c->write((addr>>16)&255);
    i2c->write((addr>>8)&255);
    i2c->write(addr&255);
    i2c->endTransmission();
}


void FPGAInterface::_setLEDRegister()   {
    i2c->beginTransmission(slave_address);
    i2c->write(LED_BASE>>24);
    i2c->write((LED_BASE>>16)&255);
    i2c->write((LED_BASE>>8)&255);
    i2c->write(LED_BASE&255);
    i2c->write(led&255);
    i2c->write(led>>8);
    i2c->endTransmission();
}