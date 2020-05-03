/*

    dmx.h
    *****************************************************************************
    * Copyright (2019 - 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
    
#ifndef _dmx_h_
#define _dmx_h_


#include "cytypes.h"
#include <stdbool.h>
#include <inttypes.h>
	
void transmitResponse(const uint8 *data, uint8 length, uint8 isbreak);
CY_ISR(DMXRxISRHandler);

enum
{
    e_DMXIDLE,
    e_DMXWAITFORSTART,
    e_DMXDATA,
    e_DMXRDM
};
	
	
#endif