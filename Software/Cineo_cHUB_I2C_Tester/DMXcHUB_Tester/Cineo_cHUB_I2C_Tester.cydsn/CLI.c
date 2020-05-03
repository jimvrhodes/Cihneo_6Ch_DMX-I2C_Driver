/*

    cli.c
    *****************************************************************************
    * Copyright (2019, 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
//
#include "main.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
//
// all of our crown jewels
extern volatile structInfo Info;
extern volatile structI2C stI2C;
//
//
extern volatile uint16 uiVCOMTransmitTimer;
//
bool volatile bWeHaveDataToVCOMOut = false;
bool volatile bWeHaveACLIResponsePending = false;
bool volatile bWeHaveAACKResponsePending = false;
//
extern volatile bool bMainWriteInfo;
//
// VCOM buffer stuff
extern volatile BYTE byCurrentVCOMBufferPtr;
extern volatile BYTE VCOMbuffer[VCOM_INPUT_BUFFER_SIZE];
extern volatile UINT uiCount;
// send now data controls
extern volatile bool bSendVCOMNow;
extern volatile BYTE bySendVCOMNowPort;     // bit mapped
//
// local CLI variables
BYTE buffer[VCOM_OUTPUT_BUFFER_SIZE];
//
extern uint16 results[NUMADCHANNELS];
extern volatile bool isADCReady;
//
extern volatile uint8 byI2CSlaveAddress;
//
// this routine expects a buffer of what the user typed, and a count
// since this is a keyboard, this should be 1 character
void ManageConfigData(void)
{
    bool bWeNeedToWriteInfo = false;
    UINT uiData;
    
    
    // parse the keyboard input stream
    switch (VCOMbuffer[A_COMMAND])
    {        
        // no packet type - simple return the value routine
        case PING:
        {
            SendACK(PINGRESPONSE, true);            
            CleanupVCOMBuffer();
        }
        break;

        // packet type A - turns on and off the ACK
        case ACK:
        {
            // wait to be done with the reception
            if(uiCount >= ACKLENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    if(VCOMbuffer[A_READWRITE] == e_WRITE)
                    {
                        // now see if we are ON or OFF
                        if(VCOMbuffer[A_DATA] == e_ACKON)
                        {
                            Info.bACKOnOff = true;
                            SendACK(ACKGOODRESPONSE, false);
                            bWeNeedToWriteInfo = true;
                        }
                        else if(VCOMbuffer[A_DATA] == e_ACKOFF)
                        {
                            Info.bACKOnOff = false;
                            SendACK(ACKGOODRESPONSE, false);
                            bWeNeedToWriteInfo = true;
                        }
                        else
                            SendACK(ACKBADRESPONSE, false);
                    }
                    // create return packet
                    else if(VCOMbuffer[A_READWRITE] == e_READ)
                    {
                        VCOMbuffer[A_DATA] = Info.bACKOnOff;
                        GenerateReadResponse();
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out
                CleanupVCOMBuffer();                
            }
        }
        break;
        
        // packet type A - turns on and off the Autosave mode
        case AUTOSAVE:
        {
            // wait to be done with the reception
            if(uiCount >= AUTOSAVELENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    if(VCOMbuffer[A_READWRITE] == e_WRITE)
                    {
                        // now see if we are ON or OFF
                        if(VCOMbuffer[A_DATA] == e_AUTOSAVEON)
                        {
                            Info.bAutoSaveOnOff = true;
                            SendACK(ACKGOODRESPONSE, false);
                            bWeNeedToWriteInfo = true;
                        }
                        else if(VCOMbuffer[A_DATA] == e_AUTOSAVEOFF)
                        {
                            Info.bAutoSaveOnOff = false;
                            SendACK(ACKGOODRESPONSE, false);
                            bWeNeedToWriteInfo = true;
                        }
                        // this will be done in the main loop
                        else if (VCOMbuffer[A_DATA] == e_AUTOSAVENOW)
                            bMainWriteInfo = true;
                        else
                            SendACK(ACKBADRESPONSE, false);
                    }
                    // create return packet
                    else if(VCOMbuffer[A_READWRITE] == e_READ)
                    {
                        VCOMbuffer[A_DATA] = Info.bAutoSaveOnOff;
                        GenerateReadResponse();
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out
                CleanupVCOMBuffer();                
            }
        }
        break;
                             
        // packet type E - read the cHUB version code
        case READCHUBVERSION:
        {
            // wait to be done with the reception
            if(uiCount >= READCHUBVERSIONLENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    // create return packet
                    if(VCOMbuffer[E_READWRITE] == e_READ)
                    {
                        VCOMbuffer[E_DATA1] = ConvertHEXToASCII(VERSIONMAJOR);
                        VCOMbuffer[E_DATA2] = ConvertHEXToASCII((VERSIONMINOR >> 8));
                        VCOMbuffer[E_DATA3] = ConvertHEXToASCII((BYTE)(VERSIONMINOR & 0xFF));
                        VCOMbuffer[E_DATA4] = ConvertHEXToASCII(VERSIONBUILD);
                        GenerateReadResponse();
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out
                CleanupVCOMBuffer();                
            }
        }
        break;
                
        // packet type D - read the cHUB NTC ADC thermistor counts
        case SENDTEMPERATURE:
        {
            // wait to be done with the reception
            if(uiCount >= SENDTEMPERATURELENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    // create return packet
                    if(VCOMbuffer[D_READWRITE] == e_READ)
                    {
                        if(isADCReady)
                        {
                            uiData = results[e_TEMP] >> 2;
                            VCOMbuffer[D_DATAH] = uiData >> 8;
                            VCOMbuffer[D_DATAL] = uiData;
                            GenerateReadResponse();
                        }
                        else
                            SendACK(ACKBADRESPONSE, true);                        
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out
                CleanupVCOMBuffer();                
            }
        }
        break;
        
        //  packet type D - Audit code
        case SETAUDITCODE:
        {
            // wait to be done with the reception
            if(uiCount >= AUDITCODELENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    if(VCOMbuffer[D_READWRITE] == e_WRITE)
                    {
                        // set the audit code now
                        Info.uiAuditNumber = (VCOMbuffer[D_DATAH] << 8) + VCOMbuffer[D_DATAL];
                                
                        SendACK(ACKGOODRESPONSE, false);
                        bMainWriteInfo = true;
                    }
                    // create return packet
                    else if(VCOMbuffer[C_READWRITE] == e_READ)
                    {
                        VCOMbuffer[D_DATAH] = (BYTE)(Info.uiAuditNumber >> 8);
                        VCOMbuffer[D_DATAL] = (BYTE)Info.uiAuditNumber;
                        GenerateReadResponse();
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out, get e_READy for next packet
                CleanupVCOMBuffer();                
            }
        }    
        break;

        // sending slider data from GUI here
        case SLIDERCH1:
        case SLIDERCH2:
        case SLIDERCH3:
        case SLIDERCH4:
        case SLIDERCH5:
        case SLIDERCH6:
        {
            // wait to be done with the reception
            if(uiCount >= SLIDERLENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    if(VCOMbuffer[D_READWRITE] == e_WRITE)
                    {
                        stI2C.uiChannelData[VCOMbuffer[A_COMMAND] - CHANNELBASE] = (VCOMbuffer[D_DATAH] << 8) + VCOMbuffer[D_DATAL];                                
                        stI2C.bySomethingChanged = true;
                        
                        SendACK(ACKGOODRESPONSE, false);
                    }
                    // create return packet
                    else if(VCOMbuffer[C_READWRITE] == e_READ)
                    {
                        // get data from driver
                        GetDataFromDriver();
                        
                        VCOMbuffer[D_DATAH] = (BYTE)(stI2C.uiChannelData[VCOMbuffer[A_COMMAND] - CHANNELBASE] >> 8);
                        VCOMbuffer[D_DATAL] = (BYTE)stI2C.uiChannelData[VCOMbuffer[A_COMMAND] - CHANNELBASE];
                        GenerateReadResponse();
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out, get e_READy for next packet
                CleanupVCOMBuffer();                
            }
        }
        break;
        
        // packet type A - send I2C Temp value to GUI
        case TEMP:
        {
            // wait to be done with the reception
            if(uiCount >= TEMPLENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    // create return packet
                    if(VCOMbuffer[A_READWRITE] == e_READ)
                    {
                        // get data from driver
                        GetDataFromDriver();
                        
                        VCOMbuffer[A_DATA] = stI2C.byTemp;
                        GenerateReadResponse();
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out
                CleanupVCOMBuffer();                
            }
        }
        break;
        
        // packet type A - send I2C voltage value to GUI
        case VOLTS:
        {
            // wait to be done with the reception
            if(uiCount >= VOLTLENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    // create return packet
                    if(VCOMbuffer[A_READWRITE] == e_READ)
                    {
                        // get data from driver
                        GetDataFromDriver();
                        
                        VCOMbuffer[A_DATA] = stI2C.byVolts;
                        GenerateReadResponse();
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out
                CleanupVCOMBuffer();                
            }
        }
        break;

        // packet type A - I2C slave address to talk to
        case SETI2CADDRESS:
        {
            // wait to be done with the reception
            if(uiCount >= SETI2CADDRESSLENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    // new address?
                    if(VCOMbuffer[A_READWRITE] == e_WRITE)
                    {
                        uint8 byTemp = VCOMbuffer[A_DATA];
                        if(byTemp >= SLAVEADDRESS && byTemp <= SLAVEADDRESS_MAX)
                        {
                            // set the address now
                            byI2CSlaveAddress = byTemp;
                                
                            SendACK(ACKGOODRESPONSE, false);
                        }
                        else
                        SendACK(ACKBADRESPONSE, true);
                    }
                    // create return packet
                    else if(VCOMbuffer[A_READWRITE] == e_READ)
                    {
                        VCOMbuffer[A_DATA] = byI2CSlaveAddress;
                        GenerateReadResponse();
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out
                CleanupVCOMBuffer();                
            }
        }
        break;
        
        // packet type E - read the cHUB version code
        case READDRVVERSION:
        {
            // wait to be done with the reception
            if(uiCount >= READDRVVERSIONLENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    // create return packet
                    if(VCOMbuffer[E_READWRITE] == e_READ)
                    {
                        // get data from driver
                        GetDataFromDriver();
                        
                        VCOMbuffer[E_DATA1] = ConvertHEXToASCII(stI2C.byVersionMajor);
                        VCOMbuffer[E_DATA2] = ConvertHEXToASCII((stI2C.byVersionMinor >> 8));
                        VCOMbuffer[E_DATA3] = ConvertHEXToASCII((BYTE)(stI2C.byVersionMinor & 0xFF));
                        VCOMbuffer[E_DATA4] = stI2C.byVersionBuild;
                        GenerateReadResponse();
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out
                CleanupVCOMBuffer();                
            }
        }
        break;
        
        //  packet type A - reset action
        case SOFT_RESET:
        {
            // wait to be done with the reception
            if(uiCount >= RESETLENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    // can only e_WRITE this one
                    if(VCOMbuffer[A_READWRITE] == e_WRITE)
                    {
                        if(VCOMbuffer[A_DATA] == e_SOFTRESET)
                            CySoftwareReset();
                        else if (VCOMbuffer[A_DATA] == e_FLASHRELOAD)
                        {
                        	LoadFromEEPROM(INFOBASE, (uint8*)&Info, sizeof(structInfo));
                            SendACK(ACKGOODRESPONSE, false);
                        }
                        else if (VCOMbuffer[A_DATA] == e_LOADEFAULTS)
                        {
                            // gets us a clean RDM store
                            LoadInfoStruct(true);
                            
                            SendACK(ACKGOODRESPONSE, false);
                        }
                        else
                            SendACK(ACKBADRESPONSE, false);
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out, get e_READy for next packet
                CleanupVCOMBuffer();                
            }
        } 
        break;
        
        // packet D - in case we need to update
        case SET_UPGRADE_FIRMWARE:
        {
            // wait to be done with the reception
            if(uiCount >= BOOTLOADLENGTH)
            {
                // see if we have a good packet
                if(CheckChecksum())
                {
                    // make sure that we are really wanting to bootload!
                    if(VCOMbuffer[D_READWRITE] == e_WRITE && VCOMbuffer[D_DATAH] == BOOTLOADBYTEH && VCOMbuffer[D_DATAL] == BOOTLOADBYTEL)
                    {
                        SendACK(ACKGOODRESPONSE, false);
                        CyDelay(1000);
                        
                        // bootload now!
//              			Bootloadable_Load();                                
                    }
                    // who knows what we got?
                    else
                        SendACK(ACKBADRESPONSE, true);
                }
                else
                    SendACK(ACKBADCHECKSUMRESPONSE, true);
                    
                // clean up and get out, get e_READy for next packet
                CleanupVCOMBuffer();                
            }
        }    
        break;
            
        // this is the whole deal
        case GET_STRUCTURE:
        break;
        
        default:
            CleanupVCOMBuffer();
        break;
    }
    
    
    // always check this in case we have auto save on
    if(bWeNeedToWriteInfo == true)
    {
        if(Info.bAutoSaveOnOff == true)
            bMainWriteInfo = true;
    }
}
//
// check the checksum!
bool CheckChecksum (void)
{
    BYTE n;
    BYTE byData = 0;
    
    // addition of all the data bytes. less the checksum
    for (n=0;n<VCOMbuffer[A_LENGTH]-1;n++)
        byData += VCOMbuffer[n];
        
    // see if they are the same        
    if(VCOMbuffer[uiCount-1] != byData)
        return(false);
    else
        return(true);
}
BYTE GenerateChecksum(void)
{
    BYTE byData = 0;
    BYTE n;
    
    // addition of all the data bytes. less the checksum
    for (n=0;n<buffer[A_LENGTH]-1;n++)
        byData += buffer[n];
        
    return(byData);
}
// send ack code, forced override capable
// 1.05 added the reflection of the command, the audit code, then the ACK response
void SendACK(BYTE byAckCode, bool bForce)
{
    BYTE n;
    static uint8_t byAckCodeStored;
    
    // make sure the port is clear, and we want to send data
    if(Info.bACKOnOff || bForce)
    {
        // store off the ACk code only once!
        if(bWeHaveDataToVCOMOut && !bWeHaveAACKResponsePending)
        {
            byAckCodeStored = byAckCode;
            bWeHaveAACKResponsePending = true;        
        }
        else if(!bWeHaveDataToVCOMOut)
            byAckCodeStored = byAckCode;
        
        // now wait for the last large transaction to finish
        if(!bWeHaveDataToVCOMOut)
        {
            // we put the response data into the original reception buffer, so just reflect it out
            for(n = 0;n< (VCOMbuffer[A_LENGTH]-1);n++)
                buffer[n] = VCOMbuffer[n];

            // send 0 if good, otherwise the ACK code
            switch(byAckCodeStored)
            {
                case ACKGOODRESPONSE:
                    buffer[A_ACK] = 0x00;
                break;
                    
                default:
                    buffer[A_ACK] = byAckCode;
                break;
            }
                    
            buffer[n] = GenerateChecksum();
            WriteLineVCOM(buffer, VCOMbuffer[A_LENGTH]);    
            
            bWeHaveAACKResponsePending = false;
        }
    }
}
//
// create the response packet with all the right stuff
//
void GenerateReadResponse(void)
{
    BYTE n;

    // bypass writing to the port if it is stuck!
    if(!bWeHaveDataToVCOMOut)
    {
        // we put the response data into the original reception buffer, so just reflect it out
        for(n = 0;n< (VCOMbuffer[A_LENGTH]-1);n++)
            buffer[n] = VCOMbuffer[n];
        buffer[A_ACK] = 0x00;
        buffer[n] = GenerateChecksum();
        WriteLineVCOM(buffer, VCOMbuffer[A_LENGTH]);

        // if we had come from this from a time when the VCOM port was busy
        bWeHaveACLIResponsePending = false;
    }
    // VCOM is busy, let main call us back later
    else
        bWeHaveACLIResponsePending = true;
}
// this is to send out data to the USB UART
// versatile routine to take any length of buffer
void WriteLineVCOM (uint8_t* buffer, uint16_t count)
{
    // this holds the bytes to get sent out, static since we buffer out at 64 bytes at a time 
    static UINT uiLastCount;
    static uint8_t Buffer[VCOM_OUTPUT_BUFFER_SIZE];
    static uint8* ptrBuffer;
    
    
    // first see if we are in the middle of a VCOM write transaction, or are starting one
    if(!bWeHaveDataToVCOMOut)
    {
        MODE1_Write(1);
                
        // start the data output process
        bWeHaveDataToVCOMOut = true;
        
        // count and buffer holder while we send out big packets > 64 bytes
        uiLastCount = count;
        memcpy(Buffer, buffer, count);
        ptrBuffer = &Buffer[0];
        
        // show that we have activity?
        LEDVCOM_Write(1);
            
        // timer to fix stalled outputs
        uiVCOMTransmitTimer = TRANSMITTIME;
    }
    
    // see if we can take any data
    if(!USBUART_CDCIsReady())
    {
        // see if we stalled long enough
        if(!uiVCOMTransmitTimer && bWeHaveDataToVCOMOut)
        {
            bWeHaveDataToVCOMOut = false;
            MODE1_Write(0);
        }
    }
    else
    {
        // timer to fix stalled outputs
        uiVCOMTransmitTimer = TRANSMITTIME;
            
        // now see how many bytes we need to send, up to 513!
        if(uiLastCount >= MAXUSBUARTBYTES)
        {
            USBUART_PutData((uint8*)ptrBuffer, MAXUSBUARTBYTES);
            uiLastCount -= MAXUSBUARTBYTES;
            ptrBuffer += MAXUSBUARTBYTES;
        }
        // make sure that we were not on the border above, and are zero
        else if(uiLastCount)
        {
            // last section of output less than 64 bytes
            USBUART_PutData(ptrBuffer, uiLastCount);
            uiLastCount = 0;
        }
        
        // are we really done?
        if(!uiLastCount)
        {
            // show that we have activity?
            LEDVCOM_Write(0);

            // we are done!
            bWeHaveDataToVCOMOut = false;
                
            MODE1_Write(0);
        }
    }
}
//
// convert two place ASCII string to one value
uint8 ConvertASCIIToHEX(char* ch)
{
    char chData[2];
    uint8 value = 0;
    
    chData[0] = *ch;
    chData[1] = *(ch+1);
    
    // see if we have one or two digits
    // look for CRLF in the stream!
    if(chData[0] && chData[1] && chData[1] != 0x0D)
    {
        value = ConvertToDecimal(chData[1]);
        value += ConvertToDecimal(chData[0]) << 4;
    }
    else
        value = ConvertToDecimal(chData[0]);
        
    return value;    
}

// convert a 2 digit HEX ASCII digit to a 8-bit value
uint8 ConvertToDecimal(char data)
{
    uint8 value = 0;
    
    // check bounds
    if(data >= '0' && data < 'A')
        // convert to HEX
        value = data - '0';
    else if(data >= 'A' && data < 'G')
        value = data - 'A' + 10;
        
    return value;
}

BYTE ConvertHEXToASCII (BYTE value)
{
    if(value <= 9)
        value += 0x30;
    else
        value += 0x37;

    return (value);
}

/* [] END OF FILE */

