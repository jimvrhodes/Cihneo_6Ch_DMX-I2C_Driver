// File: main.c
// Project: DMX and I2C Slave for Cineo ColorGenie600 board
// Current Version 1.00
//
// TODO
//
// Revision History
// 1.00A	03-03-2020	First compile DMX version
// 1.01A    05-29-2020  Added I2C control capability
//
//
/*****************************************************************************
* Copyright (2014-2020), Newton Engineering and Design Group LLC
******************************************************************************
* This software is owned by The NED Group (NED) and is protected by and subject
* to worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* NED hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* NED Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a NED product or circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of NED.

* This file contains Source Code. You may not use this file without written
* authorization from NED.
*
* Please see the License and Usage Agreement for the specific language governing rights
* and limitations under the Agreement.
*
* Disclaimer: NED MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* NED reserves the right to make changes without further notice to the
* materials described herein. NED does not assume any liability arising out
* of the application or use of any product or circuit described herein. NED
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of NED' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies NED against all charges. Use may be
* limited by and subject to the applicable NED software license agreement.
*****************************************************************************/

#include <RDM_Header.h>
#include "device.h"
#include "main.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


// instantiate our EEPROM information array
structInfo Info;
structInfo2 Info2;
structI2C stI2C;

// timers
extern uint16 ui500msTimer;
extern uint16 ui1msTimer;
extern uint16 uiChangeSaveTimer;
// DMX timers for cable on/off glitches
extern uint16 uiDMXLossTimer;
extern uint16 uiDMXPresentTimer;
// timer used to allow power to stabalize before coming ON
extern uint16 uiPowerOnDelay;
extern uint16 uiBlinkTimer;
extern uint16 uiOpHoursTimer_30M;
extern uint32 uiWriteTimer;
extern uint16 FiveSecRampUp;

// shows we are good to go - used?
volatile uint8 byBooted = 0;

// DMX Globals
volatile uint8 byDMXData[NUMCHANNELS*2] = {0};
volatile uint8 byDMXGood = 0;
uint16 uiLastDMXValue[NUMCHANNELS];


// blink the BLUE LED when we are in volt range
extern uint8 byBlink;
extern float flTemp_Scalar;

// PWM control stuff
volatile uint16 uiAPWMTarget[NUMCHANNELS] = newArray(NUMCHANNELS,BOTTOMANALOGPWM);
typeof(uiAPWMTarget) uiSPWMTarget = newArray(NUMCHANNELS,MINPWM);

float flSmoothFilter = FILTERCO;

extern volatile uint8 byLastTemp;
extern volatile uint8 byLastVolts;

volatile uint16 uiLED_ADCTemp;
volatile uint16 uiDriver_ADCTemp;
volatile uint8 byLEDTemp;
volatile uint8 byDRVTemp;

char productName[25] = {"Cineo ColorGenie600"};

float RampVal = 0.001f;
void ResolvePWMs(byte channel, float value)
{
    static float Values[NUMCHANNELS] = {0};
    float flPowerSum;
    float flOutputPowerFactor;
    
    if(!RDM_Identify)
    {
        // hold off the value for power scaling math
        Values[channel] = value;
        flPowerSum = 0;
        flOutputPowerFactor = 1;
        
        // calculate how much power we have being generated
        for(int i = 0; i < NUMCHANNELS; i++)
            flPowerSum += Values[i];

        // see if we are higher in power than we want to be - maxDiePowerScalar
        if (flPowerSum > Info2.flMaxDiePowerScalar)
            flOutputPowerFactor = Info2.flMaxDiePowerScalar/flPowerSum;
                
    	if (FiveSecRampUp)
    		RampVal = (float)(10000.000f - (float)FiveSecRampUp) / 10000.000f;
    	else 
    		RampVal = 1.000f;
    	
        // temperature compensation
    	value *= RampVal * flTemp_Scalar;

        // transistion between PWM and analog, both modes do the same

        if(value == 0.0f)
        {
    	    uiSPWMTarget[channel] = 0;
            uiAPWMTarget[channel] = 0;
        }
        else if(value < BOTTOMANALOG)
        {	
            // hold the analog at the bottom, use the PWM in this range
    	    uiSPWMTarget[channel] = (uint16)((value/BOTTOMANALOG) * (float)MAXDIGITALPWM);   
    	    uiAPWMTarget[channel] = (uint16)(BOTTOMANALOG * TOPANALOG * (float)_MaxAnalogPWM(channel));
        }    
        else
        {
            // hold the MAX PWM to the digital port, diddle the analog pin
    	    uiSPWMTarget[channel] = MAXDIGITALPWM;
            uiAPWMTarget[channel] = (uint16)(value * TOPANALOG * (float)_MaxAnalogPWM(channel) * flOutputPowerFactor);
    	}
    }
}


void (*ADIM_Set[NUMCHANNELS])(uint8_t) = { ADIM_1_Write, ADIM_2_Write, ADIM_3_Write, ADIM_4_Write, ADIM_5_Write, ADIM_6_Write };
void (*PWM_Set[NUMCHANNELS])(uint8_t) = { OFF1_Write, OFF2_Write, OFF3_Write, OFF4_Write, OFF5_Write, OFF6_Write };
void (*PINCHFET_Set[NUMCHANNELS])(uint8_t) = { PINCHFET1_Write, PINCHFET2_Write, PINCHFET3_Write, PINCHFET4_Write, PINCHFET5_Write, PINCHFET6_Write };

int main(void)
{
	uint16 uiTemp = 0;
    uint8 n;
    uint8 byVtrip = 0;
    float flSquareLawVal = 0;
    uint8 byWeRecoveredVtrip = 0;
    uint8 byDMXTestData[NUMCHANNELS*2];
    
	IsSlave = false;
    
    // no flashing please, release later before we turn on INTs
	for(uint8_t i = 0; i<NUMCHANNELS; i++)
    {
        PINCHFET_Set[i](1);
        ADIM_Set[i](0);
        PWM_Set[1](0);
    }

    // 1ms timer
    Timer_1MS_ISR_StartEx(fnTIMER_ISR);
    
	// indicator LEDs
	LED_BLUE_Write(0);
	LED_GREEN_Write(0);
	LED_RED_Write(1);
	
    CyGlobalIntEnable;
    
    ADC_Start();
    ADC_IRQ_Enable();
    ADC_StartConvert();
	
    EEPROM_Start();
	RW_EEPROMData(e_EEPROMREAD);
      
    // turn on wired DMX channel 
    DMXIsr_StartEx(DMXRxISRHandler);
	DMX_UART_Start();
    
    // read RDM data
    RW_RDM_Data(e_EEPROMREAD);
    
    // see if we have set this already
   	if(RDM.RDM_DevID[0] == VID1 && RDM.RDM_DevID[1] == VID2)
		RDM_Init(RDM.RDM_DevID, &DeviceInfo, Info.uiDMXChannel[0], false, transmitResponse);
	else
    {
		RW_RDM_Data(e_EEPROMFLUSHRESTORE);
        deviceUID[0] = VID1;
        deviceUID[1] = VID2;
        deviceUID[2] = (* (reg8 *) CYREG_FLSHID_CUST_TABLES_WAFER_NUM );
        deviceUID[3] = (* (reg8 *) CYREG_FLSHID_CUST_TABLES_X_LOC );
        deviceUID[4] = (* (reg8 *) CYREG_FLSHID_CUST_TABLES_Y_LOC );
        deviceUID[5] = (* (reg8 *) CYREG_FLSHID_CUST_TABLES_LOT_LSB );        
        RDM_Init((uint8*)deviceUID, &DeviceInfo, Info.uiDMXChannel[0], false, transmitResponse);
        sprintf((char *)RDM.stRDMIdentity, _RDM_IDENTITY);
        RW_RDM_Data(e_EEPROMWRITE);
    }
    
    // see if we are defaulted on a UID
    if(RDM.RDM_DevID[2] == 1 && RDM.RDM_DevID[3] == 2 && RDM.RDM_DevID[4] == 3 && RDM.RDM_DevID[5] == 4)
    {
        // use the device SID to define the UUID in total
        RDM.RDM_DevID[0] = VID1;
        RDM.RDM_DevID[1] = VID2;
        RDM.RDM_DevID[2] = (* (reg8 *) CYREG_FLSHID_CUST_TABLES_WAFER_NUM );
        RDM.RDM_DevID[3] = (* (reg8 *) CYREG_FLSHID_CUST_TABLES_X_LOC );
        RDM.RDM_DevID[4] = (* (reg8 *) CYREG_FLSHID_CUST_TABLES_Y_LOC );
        RDM.RDM_DevID[5] = (* (reg8 *) CYREG_FLSHID_CUST_TABLES_LOT_LSB );
        sprintf((char *)RDM.stRDMIdentity, _RDM_IDENTITY);
        RW_RDM_Data(e_EEPROMWRITE);
    }

	// wait for power to ramp up
    // power up indicator, hold solid if voltage is too low
    LED_BLUE_Write(1);
    	
    PWM_ADIM_1_Start();
    PWM_ADIM_1_WritePeriod(ANALOGPWMPERIOD);
    PWM_ADIM_1_WriteCompare1(MINPWM);
    PWM_ADIM_1_WriteCompare2(MINPWM);
    
    PWM_ADIM_2_Start();
    PWM_ADIM_2_WritePeriod(ANALOGPWMPERIOD);
    PWM_ADIM_2_WriteCompare1(MINPWM);
    PWM_ADIM_2_WriteCompare2(MINPWM);
    
    PWM_DDIM_1_Start();
    PWM_DDIM_1_WritePeriod(DIGITALPWMPERIOD);
    PWM_DDIM_1_WriteCompare1(MINPWM);
    PWM_DDIM_1_WriteCompare2(MINPWM);
    
    PWM_DDIM_2_Start();
    PWM_DDIM_2_WritePeriod(DIGITALPWMPERIOD);
    PWM_DDIM_2_WriteCompare1(MINPWM);
    PWM_DDIM_2_WriteCompare2(MINPWM);
       
    PWM_BOTHDIM5_Start();
    PWM_BOTHDIM5_WritePeriod(DIGITALPWMPERIOD);
    PWM_BOTHDIM5_WriteCompare1(MINPWM);
    PWM_BOTHDIM5_WriteCompare2(MINPWM);
    
    PWM_BOTHDIM6_Start();
    PWM_BOTHDIM6_WritePeriod(DIGITALPWMPERIOD);
    PWM_BOTHDIM6_WriteCompare1(MINPWM);
    PWM_BOTHDIM6_WriteCompare2(MINPWM);

    // filter is gonna take charge
	loopSet(uiSPWMTarget, BOTTOMDIGITALPWM, NUMCHANNELS);
	loopSet(uiAPWMTarget, BOTTOMANALOGPWM, NUMCHANNELS);
    
    // show that we have booted
	byBooted = 1;
	LED_RED_Write(0);
    
    // power on delay for power stability
//	CyDelay(500);
    
    // release the dimming control lines now
    for(uint8_t i = 0; i<NUMCHANNELS; i++)
    {
        PINCHFET_Set[i](0);
    }
    
    // start ISRs for both PWM filters
    ADIM_isr_StartEx(ADIM_ISR_Handler);
    PWMDIM_isr_StartEx(PWMDIM_ISR_Handler);
	
    // allow up to 16 slaves on the same I2C bus
    uint8 byDIPSWAddr = (~DIPSW3_Read() << 3)&0x08;
    byDIPSWAddr += (~DIPSW2_Read() << 2)&0x04;
    byDIPSWAddr += (~DIPSW1_Read() << 1)&0x02;
    byDIPSWAddr += ~DIPSW0_Read()&0x01;
    
    // for the master I2C out there to talk to us
	EZI2C_Start();
	EZI2C_SetBuffer1(sizeof(structI2C),EDITABLE,(uint8*)&stI2C);
//    EZI2C_SetAddress1(SLAVEADDRESS);
    EZI2C_SetAddress1(SLAVEADDRESS + byDIPSWAddr);
    
    stI2C.byVersionMajor = VERSIONMAJOR;
    stI2C.byVersionMinor = VERSIONMINOR;
    stI2C.byVersionBuild = VERSIONBUILD; 
    
    while(1)
	{
        //Record hours of operation in 30 min increments
		if (!uiOpHoursTimer_30M)
        {
            uiOpHoursTimer_30M = 1000;
            Info2.ulOpHalfHours++;
            
//  TODO just write the RUNTIMER here !!   WriteEEPROM(Info.bySpare, offsetof(structInfo, bySpare), bySizeOfOpHalfHours+2);
        }
		
		if (!uiWriteTimer) 
        {
			uiWriteTimer = 1000 * 60 * 10; // 10 minutes
			blSaveToInfo = true;
		}
        
        // blinks if voltage is in range, solid if below desired range
        if(byBlink)
        {
            byBlink = 0;
			
			// blink according to the status of temperature or fan
			// fastest if temp rollback is in process
			if (flTemp_Scalar < 1.0f)
				uiBlinkTimer = ONESEC/4;
			// otherwise standard blink
			else
				uiBlinkTimer = ONESEC;
            
            // show we are in Vtrip mode
            if(byVtrip)
            {
                LED_RED_Write(1);                            
            }
            else
            {
                LED_BLUE_Write(~LED_BLUE_Read());
//                LED_BLUE_Write(1);
                LED_RED_Write(0);                
                LED_GREEN_Write(0);                
            }
        }
		
        // power on delay set timer, happens on start, or transistion from DMX to POT
		if (uiPowerOnDelay)
			FiveSecRampUp = 10000;
        
        // wait for voltage to get to at least 42VDC before we output LED current
        uiTemp = ADC_Vsense_Average();
        if(uiTemp > VOLTTRIP)
        {
            // see if we are tripped so we can try to recover
            if(byVtrip)
			{
				// make sure we are above the recovery trip point for hysteresis protection
				if(uiTemp > VOLTTRIPRECOVER)
	                // then make sure that we are above the recover point so we don't blow up the board!
					byVtrip = 0;
				
                uiPowerOnDelay = RECOVERTIMER;
				byWeRecoveredVtrip = 0;
			}
			// make sure we are timed out, and voltage level is higher than we like
			else if (!uiPowerOnDelay && uiTemp > VOLTTRIPRECOVER && !byWeRecoveredVtrip)
                byVtrip = 0;
        }
        // below our desired trip point, turn off
        else
        {
            // show we have a power trip sensed
            byVtrip = 1;
			byWeRecoveredVtrip = 1;

			loopSet(uiAPWMTarget, BOTTOMANALOGPWM, NUMCHANNELS);
			loopSet(uiSPWMTarget, BOTTOMDIGITALPWM, NUMCHANNELS);
            
            for(n=0;n<NUMCHANNELS;n++)
                uiLastDMXValue[n] = 256;            
        }

        // see how hot we are, this adjusts flTemp_Scalar, which is used in the PWM isr to output scaled analog PWM values
        flTemp_Scalar = Check_Temps();
            
        // DMX stuff		
        if(byDMXGood && !uiDMXPresentTimer)
		{
            memcpy((uint8*)byDMXTestData, (uint8*)byDMXData, NUMCHANNELS*2);
            
            // see if we should store off the values in FLASH fi they changed
            // 8 bit values
            if(RDM.stRDMPersonality == 1 || RDM.stRDMPersonality == 3)
            {
    	    	// this is to keep saving the last DMX value to FLASH, but not too often, and only when there is no change
    			if (memcmp(uiLastDMXValue, (uint8*)byDMXData, NUMCHANNELS) != 0) 
                {
                	uiChangeSaveTimer = CHANGETIME;
    				blSaveOnChange = true;
    			}
            }
            // 16-bit values
            else
            {
    	    	// this is to keep saving the last DMX value to FLASH, but not too often, and only when there is no change
    			if ( uiLastDMXValue[0] != ((uint16)byDMXData[1] << 8) + byDMXData[0] || uiLastDMXValue[1] != ((uint16)byDMXData[3] << 8) + byDMXData[2] ||
                     uiLastDMXValue[2] != ((uint16)byDMXData[5] << 8) + byDMXData[4] || uiLastDMXValue[3] != ((uint16)byDMXData[7] << 8) + byDMXData[6] ||
                     uiLastDMXValue[4] != ((uint16)byDMXData[9] << 8) + byDMXData[8] || uiLastDMXValue[5] != ((uint16)byDMXData[11] << 8) + byDMXData[10] )
                {
                	uiChangeSaveTimer = CHANGETIME;
    				blSaveOnChange = true;
    			}
            }
            
            // output filter cooeficient
            if(Info.byDimFilter == e_FAST)
    		    flSmoothFilter = FILTERCO;
            else
                flSmoothFilter = SLOWFILTERCO;
            
            for(n=0;n<NUMCHANNELS;n++)
            {
                // 8-bit changes
                if(RDM.stRDMPersonality == 1 || RDM.stRDMPersonality == 3)
                {
   				    Info.SavedDMX_Value[n] = byDMXData[n];
                    uiLastDMXValue[n] = byDMXData[n];
                    
     				if(!uiLastDMXValue[n] || byVtrip)
                    {
    					uiSPWMTarget[n] = OFFDIGITALPWM;
    					uiAPWMTarget[n] = OFFANALOGPWM;
                    }
    				else
                    {
                        flSquareLawVal = (float)uiLastDMXValue[n] / (float)MAXDMX;                        
                        if(Info.byLinearMode == e_ISNONLINEAR)
                            flSquareLawVal *= flSquareLawVal;
                        
                        ResolvePWMs(n, flSquareLawVal);						
    				}
                }
                // 16 bit changes
                else
                {
   				    Info.SavedDMX_Value[n] = ((uint16)byDMXData[n*2] << 8) + byDMXData[(n*2)+1];
                    uiLastDMXValue[n] = Info.SavedDMX_Value[n];

     				if(!uiLastDMXValue[n] || byVtrip)
                    {
    					uiSPWMTarget[n] = OFFDIGITALPWM;
    					uiAPWMTarget[n] = OFFANALOGPWM;
                    }
    				else
                    {
                        flSquareLawVal = (float)uiLastDMXValue[n] / (float)MAXDMX16;
                        
                        if(Info.byLinearMode == e_ISNONLINEAR)
                            flSquareLawVal *= flSquareLawVal;
                        
                        ResolvePWMs(n, flSquareLawVal);						
    				}                    
                }                    
	        }
		}
        
		// this is to allow us to recover from a loss of DMX, wait here for the loss timer to elapse
		if (byDMXGood)
		{
			byDMXGood = 0;
			uiDMXLossTimer = DMXLOSSTIME;
		}
		// timers to catch wired DMX present and loss, reset this if we do not get a frame
		if (!uiDMXLossTimer)
			uiDMXPresentTimer = DMXPRESENTTIME;

        
    	// RDM makes this happen
    	if(blSaveToInfo)
    	{
    		blSaveToInfo = false;
    		blSaveOnChange = false;		

    		RW_EEPROMData(e_EEPROMWRITE);
    	}
    	
        // if RDM had a save event, save it off
    	if(blSaveToRDM)
    	{
    		blSaveToRDM = false;
    			
            RDM.uiDMXStartAddress = CYSWAP_ENDIAN16(DeviceInfo.uiDMXStartAddress);
                    
            if (RDM.stRDMPersonality == 1)
            {
                // set DMX addresses for 6 channels consequtively
                for(n=0;n<NUMCHANNELS;n++)
            	    Info.uiDMXChannel[n] = (CYSWAP_ENDIAN16(DeviceInfo.uiDMXStartAddress) + n);                
        	}
            else if (RDM.stRDMPersonality == 2)
            {
                // set DMX addresses for 12 channels consequtively
                for(n=0;n<NUMCHANNELS;n++)
            	    Info.uiDMXChannel[n] = (CYSWAP_ENDIAN16(DeviceInfo.uiDMXStartAddress) + (n*2));                
        	}
            else  if (RDM.stRDMPersonality == 3)
            {
                // ONE 8-bit dmx CHANNEL
                for(n=0;n<NUMCHANNELS;n++)
            	    Info.uiDMXChannel[n] = (CYSWAP_ENDIAN16(DeviceInfo.uiDMXStartAddress));                                
            }
            else
            {
                // ONE 16-bit dmx CHANNEL
                for(n=0;n<NUMCHANNELS;n++)
            	    Info.uiDMXChannel[n] = (CYSWAP_ENDIAN16(DeviceInfo.uiDMXStartAddress));                                
            }
            
    		RW_EEPROMData(e_EEPROMWRITE);
            RW_RDM_Data(e_EEPROMWRITE);
    	}
        
        // see if master I2C has something to say
        if(stI2C.bySomethingChanged == USERSAVE)
        {
            // use the upstairs routine to output data
            byDMXGood = 1;
            uiDMXPresentTimer = 0;
            
            for(n=0;n<NUMCHANNELS;n++)
            {
               byDMXData[n*2] = (uint8)(stI2C.uiChannelData[n] >> 8);
               byDMXData[(n*2)+1] = (uint8)stI2C.uiChannelData[n];
            }
                        
            // clear the request before writing
            stI2C.bySomethingChanged = I2CIDLE;
        }
        // PIT gave us something new, so recalc the checksum
        else if (stI2C.bySomethingChanged == FORCECHECKSUM)
        {
            stI2C.bySomethingChanged = I2CIDLE;
            stI2C.byVolts = byLastVolts;
            stI2C.byTemp = byLastTemp;
            CalcI2CChecksum();
        }        
    }
}

