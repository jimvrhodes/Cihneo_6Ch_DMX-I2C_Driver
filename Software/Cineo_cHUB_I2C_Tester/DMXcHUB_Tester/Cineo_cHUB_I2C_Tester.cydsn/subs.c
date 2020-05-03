/*
    subs.c
    *****************************************************************************
    * Copyright (2019 - 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
*/

#include "main.h"
#include <stdio.h>

//
// instantiate our EEPROM information array
extern structInfo Info;
extern structI2C stI2C;
//
extern volatile UINT uiStartupLEDFlashTimer;
extern volatile UINT ui1msTimer;
//
extern volatile UINT uiCount;
extern volatile BYTE VCOMInBufferPtr;
extern bool bWeHaveBufferData;
//
volatile uint8 txI2Cbuffer[sizeof(structI2C)];
extern volatile uint8 byI2CSlaveAddress;
//
uint8 WriteToMaster (uint8 byCount)
{
    uint16 uiLoops = 10000;
    
	I2C_MasterClearStatus();
	I2C_MasterWriteBuf(byI2CSlaveAddress, (uint8*)txI2Cbuffer, byCount, I2C_MODE_COMPLETE_XFER);
	while(0u == (I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT) && --uiLoops);
    
    return((uint8)uiLoops);
}
//
void ReadFromMaster(void)
{
   	uint16 uiChecksum = 0;

	// look for like checksums, and that the version is not zero (data is there)
	while(stI2C.byChecksum != uiChecksum || !stI2C.byVersionMajor)
	{
		// this is to force the slave to update the checksum realtime
		txI2Cbuffer[0] = 0;
		txI2Cbuffer[1] = FORCECHECKSUM;
		WriteToMaster (2);
        
		// settling time for driver to do checksum compute
		ui1msTimer = 5;
		while(ui1msTimer);
	
		// zero out the stI2C array
		memset(&stI2C,0,sizeof(structI2C));

		I2C_MasterClearStatus();
		I2C_MasterReadBuf(byI2CSlaveAddress, (uint8*)&stI2C, sizeof(structI2C), I2C_MODE_COMPLETE_XFER);
		while(!I2C_MasterStatus() & I2C_MSTAT_RD_CMPLT);

		// settling time
		ui1msTimer = 5;
		while(ui1msTimer);

        stI2C.bySomethingChanged = I2CIDLE;
        
		uint8* pStruct = (uint8*)&stI2C;
    	uiChecksum = 0;

		for (uint8 n=0;n<sizeof(stI2C)-1;n++)
		{
			uiChecksum += *pStruct;
			pStruct++;
		}

		uiChecksum &= 0xFF;
	}    
}
//
void GetDataFromDriver()
{
    // tell driver to write data to Flash
	txI2Cbuffer[0] = 0;		// offset address
	txI2Cbuffer[1] = USERSAVE;
    
    // if the slave is there, we will go read data
    if(WriteToMaster(2))
    {
    	// now go see who we are from the master
	    ReadFromMaster();
        LEDDMX1_Write(~LEDDMX1_Read());
    }
    // issue!
    else
    {
        I2C_MasterClearWriteBuf();
        I2C_MasterSendStop();
        LEDDMX1_Write(0);
    }
}
//
void CleanupVCOMBuffer(void)
{
    // set our count to zero to get the next packet
    uiCount = 0;
    bWeHaveBufferData = false;
}
//
void StartupLEDStrobe(BYTE byCount)
{
    BYTE n, m;
    #define NUMLEDS 7
    void (*const (WriteLEDs[NUMLEDS]))(uint8) = {LEDACT_Write, LEDERR_Write, LEDVCOM_Write, LEDDMX1_Write, LEDDMX2_Write, LEDDMX3_Write, LEDDMX4_Write};
    
    for (n=0;n<byCount;n++)
    {
        for (m=0;m<NUMLEDS;m++)
        {
            // turn off the last one
            if(m == 0)
                WriteLEDs[NUMLEDS-1](0);
            else
                WriteLEDs[m-1](0);
                
            WriteLEDs[m](1);
                
            uiStartupLEDFlashTimer = STARTUPLEDTIMER;
            while (uiStartupLEDFlashTimer);
        }    
        
        // last one needs to be off
        WriteLEDs[NUMLEDS-1](0);
    }
}

const structInfo defaultInfo = 
{
    .uiAuditNumber = 1,
    
    .bACKOnOff = true,
    .bAutoSaveOnOff = true,
    
	.byEEPROMSave1 = 0x55,
	.byEEPROMSave2 = 0xAA
};
//
// EEPROM stuff
//
void SaveInfoStruct(void)
{
    Info.byEEPROMSave1 = DATASAVE1;
    Info.byEEPROMSave2 = DATASAVE2;
	SaveToEEPROM(INFOBASE, (uint8*)&Info, sizeof(structInfo));
}
void LoadInfoStruct(bool bRebuild)
{
    // see if we are to rebuild from scratch or not
    if(!bRebuild)
	    LoadFromEEPROM(INFOBASE, (uint8*)&Info, sizeof(structInfo));
    else
        Info.byEEPROMSave1 = Info.byEEPROMSave2 = 0;
    
    // see if we are to just do defaults all the time, or try to get saved values
    if(Info.byPowerOnDefault == e_POWERONDEFAULTS)
        // easy way to just force a reload if defaults
        Info.byEEPROMSave1 = 0;
    
    // try for FLASH last writen, see if this is a fresh boot or reset
	if (Info.byEEPROMSave1 != DATASAVE1 || Info.byEEPROMSave2 != DATASAVE2)
    {    
    	memcpy((uint8*)&Info, &defaultInfo, sizeof(structInfo));
        
        // save off to EEPROM now
        SaveInfoStruct();
    }
}
//
// lowest level read and write
void LoadFromEEPROM(const uint16 EEPROMOffset, uint8 *restrict Data, const uint16 DataCount)
{
	uint16 n;
	
	for(n=0; n < DataCount; n++)
		Data[n] = EEPROM_ReadByte(EEPROMOffset + n);
}
void SaveToEEPROM(const uint16 EEPROMOffset, uint8 *restrict Data, uint16 DataCount)
{
	// start row
	uint16 Row = EEPROMOffset / CY_EEPROM_SIZEOF_ROW;
	// Calculate the start indice in the row
    uint8 Offset = EEPROMOffset % CY_EEPROM_SIZEOF_ROW;
	// create a buffer the size of a row
	uint8 SaveData[CY_EEPROM_SIZEOF_ROW];
	
	// Refresh the temperature to ensure proper EEPROM read/write speeds
	EEPROM_UpdateTemperature();
	
	// the start location does not perfectly aligned...
	if (Offset)
	{
		// Read in any data that is saved in EEPROM preceding the start
		LoadFromEEPROM(Row*CY_EEPROM_SIZEOF_ROW, SaveData, Offset);
		
		// calculate the number of bytes from the data
		uint8 BytesFromData = CY_EEPROM_SIZEOF_ROW - Offset;
		// Fill it up with data from the pointer
		memcpy(&SaveData[Offset], Data, BytesFromData);
		
		// If there were enough bytes to fill the row...
		if (Offset + DataCount >= CY_EEPROM_SIZEOF_ROW)
		{
			// Save the row
			EEPROM_Write( SaveData, Row);
			Row++;
			Offset = 0;
            
			// Remove the used bytes from the byte count
			DataCount = DataCount - BytesFromData;
			// re-indice into the data bytes
			Data = &Data[BytesFromData];
		}
	}
	
	// While there is enough data left to fill a row...
	while (DataCount >= CY_EEPROM_SIZEOF_ROW)
	{
		// Save the data directly into EEPROM
		EEPROM_Write( Data, Row);
		Row++;
		// remove the used bytes from the bytes count
		DataCount = DataCount - CY_EEPROM_SIZEOF_ROW;
		// re-indice into the data bytes
		Data = &Data[CY_EEPROM_SIZEOF_ROW];
	}
	
	// is there happens to be some data left...
	if (DataCount)
	{
		// No offset means the SaveData buffer is empty...
		if (!Offset)
		{
			// load the SaveData bytes with all the data that is left
			memcpy(SaveData, Data, DataCount);
		}
		// Load the rest of the trailing row with the data already in EEPROM
		LoadFromEEPROM(Row*CY_EEPROM_SIZEOF_ROW + DataCount + Offset, &SaveData[DataCount + Offset], CY_EEPROM_SIZEOF_ROW - DataCount - Offset);
		// write the last row
		EEPROM_Write( SaveData, Row);
	}
}
/* [] END OF FILE */
