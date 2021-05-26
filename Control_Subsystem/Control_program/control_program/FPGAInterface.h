#ifndef FPGA_INTERFACE_API_H
#define FPGA_INTERFACE_API_H

// For ESP32 

#include "Arduino.h"
#include "Wire.h"

#define COLOUR_0_OFFSET 0;
#define COLOUR_1_OFFSET 1;
#define COLOUR_2_OFFSET 2;
#define COLOUR_3_OFFSET 3;
#define COLOUR_4_OFFSET 4;

struct ColourObject {
    int detected=0;
    int angle=0;
    int distance=0;
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

    private:
        void _setAddress(long addr);

        TwoWire *i2c;
        long bus_frequency=100000;
        long base_address=0;
        int slave_address=0;

};

#endif