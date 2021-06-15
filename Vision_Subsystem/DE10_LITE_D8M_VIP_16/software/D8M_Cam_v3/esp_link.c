/*
 * esp_link.c
 *
 *  Created on: May 20, 2021
 *      Author: tansi
 */

#include "esp_link.h"

void updateColour(alt_32 base_address, alt_8 detected, alt_16 x_coord_adj, alt_16 y_coord_adj, alt_8 colour )   {
    alt_32 tmp = (x_coord_adj<<20) + (y_coord_adj<<8) + detected;
    IOWR(base_address, colour, tmp);
}


