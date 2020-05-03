/*

    iser.c
    *****************************************************************************
    * Copyright (2019 - 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/


#include "device.h"
#include "main.h"
#include <math.h>
#include <CyFlash.h>
#include <stdio.h>
#include <rdm.h>


extern structInfo Info;
extern structDeviceInfo DeviceInfo;

// this is our msec timer register set, timers here
// used for system timing
volatile uint16 ui500msTimer = FIVEHUNDREDMS;
volatile uint16 uiI2CPollTimer = 200;
volatile uint16 ui1msTimer = 0;
volatile uint16 uiChangeSaveTimer = CHANGETIME;
volatile uint16 uiTempTimer = TEMPTIMER;
volatile uint16 uiPowerOnDelay = BOOTIMER;
volatile uint16 uiBlinkTimer = FIVEHUNDREDMS;
volatile uint16 uiOpHoursTimer_30M = OPHOURSTIME;
volatile uint32 uiWriteTimer = 1000 * 60 * 10; // 10 minutes
volatile uint32 ulTempFaultTimer = _Timer_TempFault_Reset;
volatile uint16 FiveSecRampUp = 5000;
volatile uint16 USBLossTimer = 0;

// wired DMX loss and find timers
volatile uint16 uiDMXLossTimer = DMXLOSSTIME;
volatile uint16 uiDMXPresentTimer = DMXPRESENTTIME;

// timer flag
volatile uint8 byBlink = 0;

// target PWM values
extern volatile uint16 uiAPWMTarget[NUMCHANNELS];
extern volatile uint16 uiSPWMTarget[NUMCHANNELS];
extern float flSmoothFilter;

// our temperature LED power scalar
extern float flTemp_Scalar;

extern uint8 byBooted;


// timer ISR
CY_ISR(fnTIMER_ISR)
{		
	ui500msTimer--;
	if(!ui500msTimer)
	{
		ui500msTimer = FIVEHUNDREDMS;
		// operating timer 30 min intervals
	}

	if(uiOpHoursTimer_30M)
    	uiOpHoursTimer_30M--;
	
    if (FiveSecRampUp)
		FiveSecRampUp--;
	
	if(uiBlinkTimer)
		uiBlinkTimer--;
	else
        byBlink = 1;

	if(ui1msTimer)
		ui1msTimer--;
    
   	if(uiTempTimer)
		uiTempTimer--;   
    
    // wired
	if(uiDMXLossTimer)
		uiDMXLossTimer--;
	if(uiDMXPresentTimer)
		uiDMXPresentTimer--;  
    
   	if(uiChangeSaveTimer)
		uiChangeSaveTimer--;
	
	if(uiPowerOnDelay)
		uiPowerOnDelay--;
    
    if(uiI2CPollTimer)
        uiI2CPollTimer--;
    
    if(USBLossTimer)
        USBLossTimer--;
    	
	if (uiWriteTimer)
		uiWriteTimer--;
}

extern bool RDM_Identify;

CY_ISR(ADIM_ISR_Handler)
{
    uint8 n;
    static uint16 uiPWMval[NUMCHANNELS] = {};
    static float flPWMval[NUMCHANNELS] = {};
	void (*const PWM_ADIM_WriteCompare[NUMCHANNELS]) (uint16_t) = {PWM_ADIM_1_WriteCompare1, PWM_ADIM_1_WriteCompare2, PWM_ADIM_2_WriteCompare1, PWM_ADIM_2_WriteCompare2, 
                                                                   PWM_BOTHDIM5_WriteCompare1, PWM_BOTHDIM6_WriteCompare1};
	void (*const PWM_SDIM_WriteCompare[NUMCHANNELS]) (uint16_t) = {PWM_DDIM_1_WriteCompare1, PWM_DDIM_1_WriteCompare2, PWM_DDIM_2_WriteCompare1, PWM_DDIM_2_WriteCompare2,
                                                                   PWM_BOTHDIM5_WriteCompare2, PWM_BOTHDIM6_WriteCompare2};
	static bool wasIdentified = false;
	static bool byFlashState = false;
	static uint16 flashTimer = 250;
	static uint8 flashCount = 0;
    float value;

    
    if(RDM_Identify)    
    {
		wasIdentified = true;
		if(!--flashTimer)
		{
            // flash time
			flashTimer = 250;
            // toggle state
			byFlashState = !byFlashState;
            // on condition
			if(byFlashState)
			{
                value = 0.15;
                uiSPWMTarget[0] = MAXDIGITALPWM;
	            uiAPWMTarget[0] = (uint16)(value * TOPANALOG * (float)MAXANALOGPWM);
                
                uiSPWMTarget[1] = MAXDIGITALPWM;
	            uiAPWMTarget[1] = (uint16)(value * TOPANALOG * (float)MAXANALOGPWM);
                
                uiSPWMTarget[2] = MAXDIGITALPWM;
                uiAPWMTarget[2] = (uint16)(value * TOPANALOG * (float)MAXANALOGPWM);
                
                uiSPWMTarget[3] = MAXDIGITALPWM;
	            uiAPWMTarget[3] = (uint16)(value * TOPANALOG * (float)MAXANALOGPWM);

                uiSPWMTarget[4] = MAXDIGITALPWM;
	            uiAPWMTarget[4] = (uint16)(value * TOPANALOG * (float)MAXANALOGPWM);
                
                uiSPWMTarget[5] = MAXDIGITALPWM;
	            uiAPWMTarget[5] = (uint16)(value * TOPANALOG * (float)MAXANALOGPWM);
			}
            // off condition
			else
			{
                value = 0;
	            uiSPWMTarget[0] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
	            uiAPWMTarget[0] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);               
	            
                uiSPWMTarget[1] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
	            uiAPWMTarget[1] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);               
	            
                uiSPWMTarget[2] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
	            uiAPWMTarget[2] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);               
	            
                uiSPWMTarget[3] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
	            uiAPWMTarget[3] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);               

                uiSPWMTarget[4] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
	            uiAPWMTarget[4] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);
                
                uiSPWMTarget[5] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
	            uiAPWMTarget[5] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);
			}
			
            if(flashCount++ > 6)
			{
				flashTimer = 1000;
				flashCount = 0;
			}
            
            // do them both here since we are not going to use the analog routine in IDENTIFY mode
   			PWM_ADIM_WriteCompare[0](uiAPWMTarget[0]);
   			PWM_ADIM_WriteCompare[1](uiAPWMTarget[1]);
   			PWM_ADIM_WriteCompare[2](uiAPWMTarget[2]);
   			PWM_ADIM_WriteCompare[3](uiAPWMTarget[3]);
   			PWM_ADIM_WriteCompare[4](uiAPWMTarget[4]);
   			PWM_ADIM_WriteCompare[5](uiAPWMTarget[5]);
   			
            PWM_SDIM_WriteCompare[0](uiSPWMTarget[0]);
   			PWM_SDIM_WriteCompare[1](uiSPWMTarget[1]);
   			PWM_SDIM_WriteCompare[2](uiSPWMTarget[2]);
   			PWM_SDIM_WriteCompare[3](uiSPWMTarget[3]);
   			PWM_SDIM_WriteCompare[4](uiSPWMTarget[4]);
   			PWM_SDIM_WriteCompare[5](uiSPWMTarget[5]);
		}
	}
    else
    {
        // make sure we exit the RDM Identify in the OFF condition
        if(wasIdentified)
		{
			wasIdentified = false;
            value = 0;
            uiSPWMTarget[0] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
            uiAPWMTarget[0] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);               
            
            uiSPWMTarget[1] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
            uiAPWMTarget[1] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);               
            
            uiSPWMTarget[2] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
            uiAPWMTarget[2] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);               
            
            uiSPWMTarget[3] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
            uiAPWMTarget[3] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);
   			
            uiSPWMTarget[4] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
            uiAPWMTarget[4] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);
            
            uiSPWMTarget[5] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
            uiAPWMTarget[5] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)MAXANALOGPWM);

            PWM_ADIM_WriteCompare[0](uiAPWMTarget[0]);
   			PWM_ADIM_WriteCompare[1](uiAPWMTarget[1]);
   			PWM_ADIM_WriteCompare[2](uiAPWMTarget[2]);
   			PWM_ADIM_WriteCompare[3](uiAPWMTarget[3]);
   			PWM_ADIM_WriteCompare[4](uiAPWMTarget[4]);
   			PWM_ADIM_WriteCompare[5](uiAPWMTarget[5]);
   			
            PWM_SDIM_WriteCompare[0](uiSPWMTarget[0]);
   			PWM_SDIM_WriteCompare[1](uiSPWMTarget[1]);
   			PWM_SDIM_WriteCompare[2](uiSPWMTarget[2]);
   			PWM_SDIM_WriteCompare[3](uiSPWMTarget[3]);
   			PWM_SDIM_WriteCompare[4](uiSPWMTarget[4]);
   			PWM_SDIM_WriteCompare[5](uiSPWMTarget[5]);
		}
        // no RDM identify, do the normal stuff
        for (n=0;n<NUMCHANNELS;n++)
        {
            if(uiAPWMTarget[n] != uiPWMval[n])
            {
        	    flPWMval[n] = ((flPWMval[n] / flSmoothFilter) + ((float)uiAPWMTarget[n] - ((float)uiAPWMTarget[n] / flSmoothFilter)));
                
        		if(isnan(flPWMval[n]))
        		    flPWMval[n] = uiPWMval[n];

                uiPWMval[n] = flPWMval[n];
    			
    			PWM_ADIM_WriteCompare[n](uiPWMval[n]);
            }
        }
    }    
    
    // no idea why these are needed, but you only get one INT if not
    PWM_ADIM_1_ReadStatusRegister();
    PWM_ADIM_2_ReadStatusRegister();
    PWM_BOTHDIM5_ReadStatusRegister();
    PWM_BOTHDIM6_ReadStatusRegister();
}

CY_ISR(PWMDIM_ISR_Handler)
{
    uint8 n;
    static uint16 uiPWMval[NUMCHANNELS] = {};
    static float flPWMval[NUMCHANNELS] = {};
	void (*const PWM_SDIM_WriteCompare[NUMCHANNELS]) (uint16_t) = {PWM_DDIM_1_WriteCompare1, PWM_DDIM_1_WriteCompare2, PWM_DDIM_2_WriteCompare1, 
                                                                   PWM_DDIM_2_WriteCompare2, PWM_BOTHDIM5_WriteCompare2, PWM_BOTHDIM6_WriteCompare2};
      
    if(!RDM_Identify)
    {
    	// UVLO PWM control
        for (n=0;n<NUMCHANNELS;n++)
        {
            if(uiSPWMTarget[n] != uiPWMval[n])
            {
        	    flPWMval[n] = ((flPWMval[n] / flSmoothFilter) + ((float)uiSPWMTarget[n] - ((float)uiSPWMTarget[n] / flSmoothFilter)));
        		
                if(isnan(flPWMval[n]))
        		    flPWMval[n] = uiPWMval[n];
        		
                uiPWMval[n] = (uint16)flPWMval[n];
            
    			PWM_SDIM_WriteCompare[n](uiPWMval[n]);
            }
        }
    }
    
    // no idea why these are needed, but you only get one INT if not
    PWM_DDIM_1_ReadStatusRegister();
    PWM_DDIM_2_ReadStatusRegister();
    PWM_BOTHDIM5_ReadStatusRegister();
    PWM_BOTHDIM6_ReadStatusRegister();
}

