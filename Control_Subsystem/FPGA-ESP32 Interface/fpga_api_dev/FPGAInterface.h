#ifndef FPGA_INTERFACE_API_H
#define FPGA_INTERFACE_API_H

// For ESP32 

#include "Arduino.h"
#include "Wire.h"

#ifndef LED_BASE
    #define LED_BASE 0x1000
#endif

#define COLOUR_0_OFFSET 0;
#define COLOUR_1_OFFSET 1;
#define COLOUR_2_OFFSET 2;
#define COLOUR_3_OFFSET 3;
#define COLOUR_4_OFFSET 4;

struct ColourObject {
    int detected=0;
    int x=0;
    int y=0;
};

class FPGAInterface {
    public:
        FPGAInterface(TwoWire *tw);
        void begin();
        void begin(gpio_num_t sda, gpio_num_t scl);
        void setBusFrequency(long freq);
        void setSlaveAddress(int saddr);
        void setBaseAddress(long ba);
        ColourObject readByIndex(int index);

        void writeLED(int n, int value);
        void toggleLED(int n);
        void offLEDs();
        void onLEDs();

    private:
        void _setAddress(long addr);
        void _setLEDRegister();

        TwoWire *i2c;
        long bus_frequency=100000;
        long base_address=0;
        int slave_address=0;

        int led=0;
};

#endif