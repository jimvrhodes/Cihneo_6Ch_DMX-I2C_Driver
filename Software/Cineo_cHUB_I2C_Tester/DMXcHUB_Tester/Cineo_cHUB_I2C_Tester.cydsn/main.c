/*****************************************************************************
 
 Cineo ColorGenie Tester Using DMXcHUB as a USBUART and I2C Master

 Revision History
    1.00B0		05-01-2019  Started, released 10-16-2019
    
    
TODO

1) 
2) 
3) 


******************************************************************************
* Copyright (2019, 2020), Newton Engineering and Design Group LLC
******************************************************************************
* This software is owned by NED Group LLC (NED) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
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
//
//
#include "main.h"
#include "cytypes.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//
// timers
extern volatile uint16 uiLEDBlinkTimer;
extern volatile uint16 uiVCOMReceptionTimer;
extern volatile uint16 uiVCOMNoMoreTimer;
extern volatile uint16 uiReadDriverI2CTimer;
//
// CLI VCOM handler variables
extern bool volatile bWeHaveDataToVCOMOut;
extern bool volatile bWeHaveACLIResponsePending;
extern bool volatile bWeHaveAACKResponsePending;
extern BYTE buffer[VCOM_OUTPUT_BUFFER_SIZE];
//
structInfo Info;
structI2C stI2C;
extern volatile uint8 txI2Cbuffer[sizeof(structI2C)];
volatile uint8 byI2CSlaveAddress = SLAVEADDRESS;
//
// VCOM input buffer stuff
volatile UINT uiCount = 0;
volatile BYTE VCOMbuffer[VCOM_INPUT_BUFFER_SIZE] = {0};
//
volatile bool bMainWriteInfo = false;
volatile bool bWeAreConected = false;
//
//
// global for debug
UINT uiBufferCount;
bool bWeHaveBufferData = false;
//
//
int main()
{
    BYTE byToggle = 0;;
    BYTE* ptrVCOM;
    //    
    //
    // all of our indicator LEDs
    LEDACT_Write(1);
    LEDERR_Write(0);
    LEDVCOM_Write(0);
    LEDDMX1_Write(0);
    LEDDMX2_Write(0);
    LEDDMX3_Write(0);
    LEDDMX4_Write(0);
    //
    MODE1_Write(1);
    // interrupt timer    
    CyIntSetSysVector((SysTick_IRQn + 16), fnTIMER_ISR);
	SysTick_Config(48000000/(DIVIDER*1000));
    //
    // startup and get EEPROM data
	EEPROM_Start();
	LoadInfoStruct(false);
    //
    // Enable Global Interrupts
    CyGlobalIntEnable;               
    //
    StartupLEDStrobe(STARTUPLEDCYCLES);
    //
    // Start USBFS Operation with 3V operation
    USBUART_Start(0u, USBUART_5V_OPERATION);
    // Wait for Device to enumerate
    while(!USBUART_GetConfiguration());
    //
    // Enumeration is done, enable OUT endpoint for receive data from Host
    USBUART_CDC_Init();
    //
    // Start the ADC
	ADC_SAR_Seq_Start();
	ADC_SAR_Seq_IRQ_StartEx(ADCIRQHandler);
	ADC_SAR_Seq_StartConvert();
    //
    // startup the I2C Communication bus
 	I2C_Start();
    stI2C.bySomethingChanged = I2CIDLE;
    //
    //
    // Main Loop
    while(1)
    {
        if(!uiLEDBlinkTimer)
        {
            uiLEDBlinkTimer = 500;
            
            if(byToggle)
            {
                byToggle = 0;
                LEDACT_Write(0);
            }
            else
            {   byToggle = 1;
                LEDACT_Write(1);
            }
        }
        //
        // These are all the buffer handlers in case the USB serial port is busy, queue them up...
        //
        // see if we need to go see if the VCOm can take more data or not, in 64 byte chunks
        if(bWeHaveDataToVCOMOut)
            WriteLineVCOM(buffer,0);
        //
        // see if we have a CLI response pending
        if(bWeHaveACLIResponsePending)
            GenerateReadResponse();
        //    
        // reflection of the command, the audit code, then the ACK response
        if(bWeHaveAACKResponsePending)
            SendACK(0x00, true);
        //    
        // see if we have anything from the USB VCOM host
        if(USBUART_DataIsReady() != 0u)
        {
            // get them one buffer at a time so we can control this, we don't know how long the string coming in is,
            //  so just get them until we are good with the VCOM RX count, then handle the message below
            uiBufferCount = USBUART_GetCount();

            // pointer to where we left off the last round of fetching data
            ptrVCOM = (BYTE*)&VCOMbuffer[uiCount];
    
            // make sure we are not getting more than our buffer
            if(uiCount+uiBufferCount > VCOM_INPUT_BUFFER_SIZE)
            {
                USBUART_GetData(ptrVCOM, (uiCount+uiBufferCount-513));
                uiCount = VCOM_INPUT_BUFFER_SIZE;    
            }
            // normal append of current data to reception buffer
            else
            {
                USBUART_GetAll(ptrVCOM);
                uiCount += uiBufferCount;
            }
            
            // this keeps us from getting stuck on a zero byte input buffer count
            if(uiBufferCount)
                // reset our reception timer, kills packets that don't finish
                uiVCOMReceptionTimer = RECEPTIONTIME;
            
            // to show that there was some activity
            bWeHaveBufferData = true;
            
            // show on VCOM LED that we have activity?
            LEDVCOM_Write(~LEDVCOM_Read()); 
                           
            // call CLI switch
            ManageConfigData();
            
            CleanupVCOMBuffer();
            
            bWeAreConected = true;
        }
        //        
        // see if our packet from the VCOM failed short
        if(!uiVCOMReceptionTimer && bWeHaveBufferData)
        {
            USBUART_CDC_Init();
            
            CleanupVCOMBuffer();                    
            LEDVCOM_Write(0);
        }
        //
        // these are all flags that could have happened somewhere else, we don't want to send data to the host
        //   unless there is nothing else going on
        // we also don't want to be writing EEPROM while something else is going on
        //
        // see if we changed something in our CLI function
        if(bMainWriteInfo)
        {
            SaveInfoStruct();            
            bMainWriteInfo = false;
        }
        //        
        // see if we have data to send to driver
        if(stI2C.bySomethingChanged != I2CIDLE && bWeAreConected)
        {
            LEDDMX2_Write(~LEDDMX2_Read());
            
            // send the data first
            memcpy((uint8*)&txI2Cbuffer[1],(uint8*)&stI2C, EDITABLE);                    
			txI2Cbuffer[0] = 0;		// offset address
			txI2Cbuffer[1] = 0;        // byUserSave
            
			if(!WriteToMaster(EDITABLE))
            {
                 // issue!
                I2C_MasterClearWriteBuf();
                I2C_MasterSendStop();
                LEDDMX2_Write(0);
            }
            else
            {
                // tell driver to write data to Flash
    			txI2Cbuffer[0] = 0;		// offset address
    			txI2Cbuffer[1] = USERSAVE;
                if(!WriteToMaster(2))
                {
                     // issue!
                    I2C_MasterClearWriteBuf();
                    I2C_MasterSendStop();
                    LEDDMX2_Write(0);
                }
                else
                    stI2C.bySomethingChanged = I2CIDLE;
            }
        }
        
        // fetch temp and voltage every interval
//        if(!uiReadDriverI2CTimer && bWeAreConected)
//        {
//            uiReadDriverI2CTimer = 100;
//            GetDataFromDriver();
//        }
    }   
}
//
/* [] END OF FILE */

    