/*
    isr.c
    *****************************************************************************
    * Copyright (2019) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
*/


#include "main.h"


// structures
extern structInfo Info;
//
extern volatile UINT count;

// this is our msec timer register set, timers here
// used for system timing
volatile uint16 ui1msTimer = 0;
volatile uint16 uiLEDBlinkTimer;
volatile uint16 uiVCOMReceptionTimer;
volatile uint16 uiVCOMTransmitTimer;
volatile uint16 uiStartupLEDFlashTimer;
volatile uint16 uiReadDriverI2CTimer;
//
//
uint16 results[NUMADCHANNELS] = {0};
volatile bool isADCReady = false;
//
CY_ISR(ADCIRQHandler)
{
    // grab and store
	for(uint8 i=0; i<sizeof(results)/sizeof(results[0]); i++)
		results[i] = ADC_SAR_Seq_GetResult16(i);
            
    // show we have done this
	isADCReady = true;
}
//
// timer ISR in uSec
//
extern BYTE buffer[VCOM_OUTPUT_BUFFER_SIZE];
extern bool volatile bWeHaveDataToVCOMOut;

CY_ISR(fnTIMER_ISR)
{
    static UINT divider = DIVIDER;
    
            
    if(uiVCOMReceptionTimer)
        uiVCOMReceptionTimer--;
    
    if(uiVCOMTransmitTimer)
        uiVCOMTransmitTimer--;
    
    // these are mSec increments
    if(!(--divider) )
    {
        // reseed
        divider = DIVIDER;
        
    	if(ui1msTimer)
    		ui1msTimer--;
        
        if(uiLEDBlinkTimer)
            uiLEDBlinkTimer--;
        
        if(uiStartupLEDFlashTimer)
            uiStartupLEDFlashTimer--;
        
//        if(uiReadDriverI2CTimer)
//            uiReadDriverI2CTimer--;
    }
}

