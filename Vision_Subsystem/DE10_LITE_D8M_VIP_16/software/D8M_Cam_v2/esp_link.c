/*
 * esp_link.c
 *
 *  Created on: May 20, 2021
 *      Author: tansi
 */

#include "esp_link.h"

void updateColour(alt_32 base_address, alt_8 detected, alt_8 angle, alt_16 distance, alt_8 colour )   {
    alt_32 tmp = (distance<<16) + (angle<<8) + detected;
    IOWR(base_address, colour, tmp);
}


