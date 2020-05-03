/*

    subs.c
    *****************************************************************************
    * Copyright (2019 - 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/


#include "device.h"
#include "main.h"
#include <math.h>
#include <stdio.h>
#include <rdm.h>


extern structInfo Info;
extern structInfo2 Info2;
extern structDeviceInfo DeviceInfo;
extern structI2C stI2C;

BYTE guiData[78];

// our temperature power scalar, used in the PWM isr to scale the output
volatile float flTemp_Scalar = 1.0f;

// timers
extern uint16 uiTempTimer;
extern uint32 ulTempFaultTimer;

// reporting our temps
extern volatile uint16 uiLED_ADCTemp;
extern volatile uint16 uiDriver_ADCTemp;
extern volatile uint8 byLEDTemp;
extern volatile uint8 byDRVTemp;

extern bool blSaveToInfo;

volatile uint16 uiADCAverage_TEMP[COUNT] = {TEMPTRIPTHRESHOLD};
volatile uint16 uiADCAverage_EXTTEMP[COUNT] = {TEMPTRIPTHRESHOLD};
volatile uint16 uiADCAverage_VSENSE[COUNT] = {VOLTTRIP + 100};
//
volatile uint8 byLastTemp;
volatile uint8 byLastVolts;
//
//
void CalcI2CChecksum()
{
    uint8* pStruct = (uint8*)&stI2C;
    uint16 uiChecksum = 0;
	for (uint8 n=0;n<sizeof(structI2C)-1;n++)
	{
		uiChecksum += *pStruct;
		pStruct++;
	}
    
    stI2C.byChecksum = (uint8)uiChecksum;
}
//
//
uint8 CalcTempInC (uint16 uiNTCADCValue)
{   
    #define NTCLOW   (float)1939
    #define NTCHIGH  (float)1115
    #define LOWTEMP  (float)27
    #define HIGHTEMP (float)43
    
    float flTemp1 = (float)(uiNTCADCValue)-NTCLOW;
    float flTemp2 = (LOWTEMP-HIGHTEMP) / (NTCLOW - NTCHIGH);
    return ((uint8)((flTemp1 * flTemp2) + LOWTEMP));
}
uint8 CalcEXTTempInC (uint16 uiNTCADCValue)
{
    // these are all numbers that were given to me by Brian Moon 4-28-2020
    #define NTCLOWX   (float)1710.0f
    #define NTCHIGHX  (float)984.0f
    #define LOWTEMPX  (float)23.889f
    #define HIGHTEMPX (float)49.0f
    
    float flTemp1 = (float)(uiNTCADCValue)-NTCLOWX;
    float flTemp2 = (LOWTEMPX-HIGHTEMPX) / (NTCLOWX - NTCHIGHX);
    return ((uint8)((flTemp1 * flTemp2) + LOWTEMPX));
}
//
// see what is up with the internal NTC sensor
float Check_Temps()
{
	static bool tempTrigger = false;
	static float currentTemp = 100.0f;
    
	if (!uiTempTimer)  
	{
		uiTempTimer = Info.uiTempTimer;
		
		// read on-board temp
		uiDriver_ADCTemp = ADC_INTTemp_Average();
        byDRVTemp = CalcTempInC(uiDriver_ADCTemp);
        byLastTemp = byDRVTemp;
        
        // read external LED temp
		uiLED_ADCTemp = ADC_EXTTemp_Average();
        byLEDTemp = CalcEXTTempInC(uiLED_ADCTemp);
        
        // power rollback
		if((uiDriver_ADCTemp < Info.uiTempTrip) || ( uiLED_ADCTemp && (uiLED_ADCTemp < Info2.uiExtTempTrip)) )
        {
			if (!tempTrigger) {
				tempTrigger = true;
				uiTempFaultCount++;
			}
			
			if (!isInTempFault){
				isInTempFault = true;
				ulTempFaultTimer = _Timer_TempFault_Reset;
			}
			
            if(currentTemp > Info.flBottomTempRatio) {
				if ((currentTemp - Info.flTempRampDown) < Info.flBottomTempRatio)
					currentTemp = Info.flBottomTempRatio;
				else
                	currentTemp -= Info.flTempRampDown;
			}
        }
        else
        {
			tempTrigger = false;
			
            if(currentTemp < DEFAULTEMPSCALAR) {
				if ((currentTemp + Info.flTempRampUp) > DEFAULTEMPSCALAR)
					currentTemp = DEFAULTEMPSCALAR;
				else
                	currentTemp += Info.flTempRampUp;
			}
        }
		
		if (!ulTempFaultTimer && isInTempFault) {
			Info.ulTempCount = uiTempFaultCount;
			isInTempFault = false;
			blSaveToInfo = true;
		}
	}
	
	if (currentTemp > 100.0f)
        currentTemp = 100.0f;
    
	return currentTemp/100.0f;
}


uint16 ADC_EXTTemp_Average(void)
{
	uint32 uiResult = 0;
    uint8 n;
    
    for(n=0;n<COUNT;n++)
		uiResult += uiADCAverage_EXTTEMP[n];
			
	return(uiResult/COUNT);
}

uint16 ADC_INTTemp_Average(void)
{
	uint32 uiResult = 0;
    uint8 n;
    
    for(n=0;n<COUNT;n++)
		uiResult += uiADCAverage_TEMP[n];
			
	return(uiResult/COUNT);
}

uint16 ADC_Vsense_Average(void)
{
	uint32 uiResult = 0;
    uint8 n;
    
    for(n=0;n<COUNT;n++)
		uiResult += uiADCAverage_VSENSE[n];
    
    uiResult = uiResult/COUNT;

    // 68K and 604K divider
    byLastVolts = (uint8)((uiResult * (5.0f/(float)MAXADC)) / (68000.0f/(640000.0f+68000.0f)));
        
	return(uiResult);
}


void LoadFromEEPROM( const uint16 EEPROMOffset, uint8 *restrict Data, const uint16 DataCount)
{
	uint16 n;
	for(n=0; n < DataCount; n++)
	{
		Data[n] = EEPROM_ReadByte(EEPROMOffset + n);
	}
}
void SaveToEEPROM( const uint16 EEPROMOffset, uint8 *restrict Data, uint16 DataCount)
{
	uint16 Row = EEPROMOffset / CY_EEPROM_SIZEOF_ROW;
	uint8 Offset = EEPROMOffset % CY_EEPROM_SIZEOF_ROW;
	uint8 SaveData[CY_EEPROM_SIZEOF_ROW];
	
	EEPROM_UpdateTemperature();
	
	if (Offset)
	{
		/*Read in any data that is saved in EEPROM preceding the start point*/
		LoadFromEEPROM(Row*CY_EEPROM_SIZEOF_ROW, SaveData, Offset);
		/*Calculate the number of bytes from the data*/
		uint8 BytesFromData = CY_EEPROM_SIZEOF_ROW - Offset;
		/*Fill it up with data from the pointer*/
		memcpy(&SaveData[Offset], Data, BytesFromData);
		
		/*If there are enough bytes to not need to load trailing data...*/
		if (Offset + DataCount >= CY_EEPROM_SIZEOF_ROW)
		{
			/*Save this row*/
			EEPROM_Write( SaveData, Row);
			Row++;
			Offset = 0;
			/*Remove that many bytes from the count and re-index into data*/
			DataCount = DataCount - BytesFromData;
			Data = &Data[BytesFromData];
		}
	}
	
	while (DataCount >= CY_EEPROM_SIZEOF_ROW)
	{
		EEPROM_Write( Data, Row);
		Row++;
		DataCount = DataCount - CY_EEPROM_SIZEOF_ROW;
		Data = &Data[CY_EEPROM_SIZEOF_ROW];
	}
	
	if (DataCount)
	{
		if (!Offset)
		{
			memcpy(SaveData, Data, DataCount);
		}
		LoadFromEEPROM(Row*CY_EEPROM_SIZEOF_ROW + DataCount + Offset, &SaveData[DataCount + Offset], CY_EEPROM_SIZEOF_ROW - DataCount - Offset);
		EEPROM_Write( SaveData, Row);
	}
}


const structInfo defaultInfo = 
{
	.uiDMXChannel[0] = 1u,
	.uiDMXChannel[1] = 2u,
	.uiDMXChannel[2] = 3u,
	.uiDMXChannel[3] = 4u,
	.uiDMXChannel[4] = 5u,
	.uiDMXChannel[5] = 6u,
	
    .byCurrentSet[0] = CHANNEL1CURRENT,
    .byCurrentSet[1] = CHANNEL2CURRENT,
    .byCurrentSet[2] = CHANNEL3CURRENT,
    .byCurrentSet[3] = CHANNEL4CURRENT,
    .byCurrentSet[4] = CHANNEL5CURRENT,
    .byCurrentSet[5] = CHANNEL6CURRENT,
    .uiMaxCurrent = MAXCURRENT,
	.byLinearMode = e_ISLINEAR,
	.byDimFilter = e_FAST,
    
    .uiTempTrip = TEMPTRIPTHRESHOLD,
    
    .flBottomTempRatio = MINTEMPSCALAR,
    .flTempRampUp = TEMPSCALARADD,
    .flTempRampDown = TEMPSCALARMINUS,
    .uiTempTimer = TEMPTIMER,
	
    .SavedDMX_Value[0 ... 5] = 0x00,
    
	.byEEPROMSave1 = 0x55,
	.byEEPROMSave2 = 0xAA,
};

const structInfo2 defaultInfo2 = {
    
    .flMaxDiePowerScalar = NUMCHANNELS, 
    .uiExtTempTrip = EXTTEMPTRIP,
    .ulOpHalfHours = 0,
    
    .byEEPROMSave1 = 0x55,
	.byEEPROMSave2 = 0xAA,
};



// ----------START EEPROM FLASH BLOCK ----------

void RW_EEPROMData(uint8 RW)
{
	if (RW == e_EEPROMWRITE)
    {
          SaveToEEPROM(0x10, (uint8*)&Info, sizeof(structInfo));
          SaveToEEPROM(0x100, (uint8*)&Info2, sizeof(structInfo2));
    } 
    else if (RW == e_EEPROMREAD)
    {	
    	LoadFromEEPROM(0x10, (uint8*)&Info, sizeof(structInfo));
        
        if (Info.byEEPROMSave1 != 0x55 || Info.byEEPROMSave2 != 0xAA)
        {
			memcpy((uint8 *)&Info, (uint8 *)&defaultInfo, sizeof(structInfo));
            SaveToEEPROM(0x10, (uint8*)&Info, sizeof(structInfo));
        }
        
    	LoadFromEEPROM(0x100, (uint8*)&Info2, sizeof(structInfo2));

		if (Info2.byEEPROMSave1 != 0x55 || Info2.byEEPROMSave2 != 0xAA)
        {
			memcpy((uint8 *)&Info2, (uint8 *)&defaultInfo2, sizeof(structInfo2));
            SaveToEEPROM(0x100, (uint8*)&Info2, sizeof(structInfo2));
		}
	}
}


//extern structInfo Info;
extern structRDM RDM;

const structRDM defaultRDM = {
    .stRDMIdentity = "Cineo 6 Channel",
	.stRDMManufacturer = "Cineo",
	.stRDMModel = "C6CHAN",
	.RDM_DevID = {VID1, VID2, 0x01, 0x02, 0x03, 0x04},
    .stRDMPersonality = 2,      // 16 bit is default
    .uiDMXStartAddress = 1,
};

void RW_RDM_Data(uint8 byRW)
{
    if (byRW == e_EEPROMWRITE)
    {
          SaveToEEPROM(0x200, (uint8*)&RDM, sizeof(structRDM));
    }
    else if (byRW == e_EEPROMREAD || byRW == e_EEPROMFLUSHRESTORE)
    {
    	LoadFromEEPROM(0x200, (uint8*)&RDM, sizeof(RDM));
        
        if (byRW == e_EEPROMFLUSHRESTORE)
        {									// RDM RESTORE
			memcpy((uint8*)&RDM, (uint8*)&defaultRDM, sizeof(structRDM));
            SaveToEEPROM(0x200, (uint8*)&RDM, sizeof(structRDM));
        }
    }
}

