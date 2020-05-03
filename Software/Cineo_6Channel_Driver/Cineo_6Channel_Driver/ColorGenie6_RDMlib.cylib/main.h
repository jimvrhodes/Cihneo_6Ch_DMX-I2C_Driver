/*

    main.h
    *****************************************************************************
    * Copyright (2019 - 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
    
#ifndef _main_h_
#define _main_h_



#include "cytypes.h"
#include <stdbool.h>
#include <inttypes.h>


#define NUMADCCHANNELS 4
    
// DEBUG!
//#define USB_ENABLED	0
    
typedef uint16 UINT;
typedef uint8 BYTE;
typedef uint8 byte;
typedef union
{
   UINT u16;
   struct
   {
        BYTE u8l;
        BYTE u8h;
   };

} uint16or8;


#define VERSIONMAJOR 1
#define VERSIONMINOR 1
#define VERSIONBUILD 'A'

// Cineo VID
#define VID1 0x1A
#define VID2 0x0D

// smoothing filter cooeficients
//#define FILTERCO 1.01f
//#define FILTERCO 1.005f
//#define NOFILTERCO 1.50f
//#define FILTERCO 1.50f
#define FILTERCO 1.25f
#define SLOWFILTERCO 1.01f
//
//
// prototypes
void ProcessValues(float Intensity, float CCT, float Blend, float Color);
void EEPROM_UpdateState();
void RW_EEPROMData(uint8);
void RW_RDM_Data(uint8 byRW);
uint16 ADC_EXTTemp_Average(void);
uint16 ADC_INTTemp_Average(void);
uint16 ADC_Vsense_Average(void);
uint8 CalcTempInC (uint16 uiNTCADCValue);
uint8 CalcEXTTempInC (uint16 uiNTCADCValue);
float Check_Temps(void);
void CalcI2CChecksum(void);
//
#define MAP_VALUE(value, in_min, in_max, out_min, out_max) ((int16)((int16)value - (int16)in_min) * (int16)((int16)out_max - (int16)out_min) / (int16)((int16)in_max - (int16)in_min) + (int16)out_min)
//
CY_ISR_PROTO(fnTIMER_ISR);
CY_ISR_PROTO(DMXRxISRHandler);
CY_ISR_PROTO(PWMDIM_ISR_Handler);
CY_ISR_PROTO(ADIM_ISR_Handler);
//
// number of channels
#define NUMCHANNELS 6
// ADC sample buffer length
#define COUNT 50
//
// this is where we shut off the output to the LEDs, using the UVLO pin
#define VOLTTRIP 3100                       // 42VDC
#define VOLTTRIPRECOVER VOLTTRIP + 300      // 44VDC
//
// all the PWM values that matter
#define MINPWM 0
#define BOTPWM 0
#define HARDOFFPWM 0
#define MAXPWM 65535
#define ANALOGPWMPERIOD 65535
#define DIGITALPWMPERIOD 65535
#define MAXANALOGPWM 65535
#define MAXDIGITALPWM 65535
//
// for defining the actual current to the LED string
#define MAXCURRENT  	2000u       // this is what the hardware has set with the CS resistor
#define CHANNELCURRENTMAX 2000u
#define CHANNEL1CURRENT CHANNELCURRENTMAX
#define CHANNEL2CURRENT CHANNELCURRENTMAX
#define CHANNEL3CURRENT CHANNELCURRENTMAX
#define CHANNEL4CURRENT CHANNELCURRENTMAX
#define CHANNEL5CURRENT CHANNELCURRENTMAX
#define CHANNEL6CURRENT CHANNELCURRENTMAX
#define SATURATESCALAR (0.98f)		// this sets the top for all channels

#define _MaxAnalogPWM(x)	(uint16_t)(MAXPWM *((float)Info.byCurrentSet[x]/Info.uiMaxCurrent))

// dimming limits
#define BOTTOMANALOG (float)(5.0f/255.0f)
//#define BOTTOMANALOG (float)(25.0f/255.0f)
#define TOPANALOG SATURATESCALAR
#define BOTTOMANALOGPWM (uint16)(BOTTOMANALOG*(float)TOPANALOG)
#define OFFANALOGPWM BOTTOMANALOGPWM
#define OFFDIGITALPWM MINPWM
#define BOTTOMDIGITALPWM OFFDIGITALPWM

// TEMP stuff - 4095 referenced
#define MAXADC 4095

#define _Timer_TempFault_Reset	(1000 * 60 * 10) // 30min

#define TEMPTRIPTHRESHOLD 1100	// this is 50C at NTC, 85C at LEDs

#define EXTTEMPTRIP 450  // external temperature set approx. 85C based on 15kohm NTC divider with 510 ohm series resistance to PSoC

// temp test interval
#define TEMPTIMER 3000
// timer for fixing the temp issues - x500ms times
#define DEFAULTEMPSCALAR 100.0f
#define MINTEMPSCALAR (float)10.0f
// add value for power scaling due to temp issues
#define TEMPSCALARADD (float)0.5f	// add this to start getting back to a good temp
// add value for power scaling due to temp issues
#define TEMPSCALARMINUS (float)0.5f	// subtract this to start getting back to a good temp

// 1ms timer rate timer define, that clocks at a 1ms rate
#define FIVEHUNDREDMS 500		// this many TIMER8 ints
#define ONESEC 1000		// this many TIMER8 ints
#define HUNDREDMS 100	// we get to the SystemTimer routine every 10ms

// these are the power on and power droop recover timers
#define BOOTIMER 2000
#define RECOVERTIMER 1000

// operating timers
#define OPHOURSTIME (1800000/FIVEHUNDREDMS) // 30 mins

// something changed and stopped, so write to FLASH timer
#define CHANGETIME 3000
// 1.04 timers for loss, and recovery of DMX signal
#define DMXLOSSTIME 500
#define DMXPRESENTTIME 1000
#define MAXDMX 255
#define MAXDMX16 65535

// this is for EEPROM writing
#define CLEAR 0
#define SAVEIT 1

#define ADCSCALAR (float)((float)5/(float)4095)

enum
{
    e_1CHANNEL,
    e_2CHANNEL
};

enum
{
	e_TEMP,
    e_VSENSE,
	e_EXTTEMP
};

// EEPROM stuff
enum
{
  e_EEPROMWRITE = 0,
  e_EEPROMREAD = 1,
  e_EEPROMFLUSHRESTORE = 2
};

enum
{
    e_ISLINEAR,
    e_ISNONLINEAR
};

enum
{
    e_FAST,
    e_SLOW
};
//
//
// I2C stuff
#define NUMCHANNELS 6
#define CHANNELBASE SLIDERCH1
#define SLAVEADDRESS 22
#define USERSAVE 0xA5
#define FORCECHECKSUM 0xCA
#define SETDEFAULTS 0xCC
#define I2CIDLE 0
#define EDITABLE 20
//
// this is the I2C structure
//
typedef struct __attribute__ ((__packed__))
{
    uint8 bySomethingChanged;
    
    uint16 uiChannelData[NUMCHANNELS];
    
    uint8 byTemp;
    uint8 byVolts;
    
    uint8 bySpare[5];
    
    uint8 byVersionMajor;
    uint8 byVersionMinor;
    uint8 byVersionBuild;
    
    uint8 byChecksum;
    
} structI2C;
//
// this is the EEPROM save structure
//
typedef struct __attribute__ ((__packed__))
{
    // save data to display controller
	uint16 uiDMXChannel[NUMCHANNELS];     	// DMX start Address for each channel
	uint16 byCurrentSet[NUMCHANNELS];		// number between 0 and 5000 (0-5 Amps)
    uint16 uiMaxCurrent;
	uint8 byLinearMode;	               	// linear or square law dimming, 0 = linear, 1 = squarelaw																28bytes
	uint8 byDimFilter;	               	// fast or slow, 0 = fast, 1 = intertia																28bytes
    
    // temperature in analog conversion readings, we use the temp sensor to make these decisions
    uint16 uiTempTrip;          // this is the analog value to start turning down the power to the LED (flTemp_Scalar)                                  40bytes
    
    float flBottomTempRatio;    // this is the lowest dim level to stop when we are in over temp (flTemp_Scalar)                                        44bytes
    float flTempRampUp;         // this is how much we bump back the flTemp_Scalar value until we hit 1.00                                              48bytes
    float flTempRampDown;       // this is how much we bump back the flTemp_Scalar value until we hit flBottomTempRatio                                 52bytes
    uint16 uiTempTimer;         // now fast in ms that we want to check the over temp sensor and do the add or subtract of flTemp_Scalar                54bytes

	uint32 ulTempCount;			//																														58bytes
    
    uint16 SavedDMX_Value[NUMCHANNELS];    //                                                                                                                      62bytes
	 
	//
    // these are the items that cannot be changed by the programming head
    // these are the items that define the hardware that is not editable
    //  note that this is part of the big structure that the progrmaming head
    //  will read later, and is not editable by the programming head
    //
    BYTE byEEPROMSave1;				// these are used to see if the EEPROM is good or not (2)                                                           65bytes
    BYTE byEEPROMSave2;             //                                                                                                                  66bytes
} structInfo;


typedef struct __attribute__ ((__packed__))
{
    float flMaxDiePowerScalar;       // Maximum current level/die                                                                                             4bytes
    uint16 uiExtTempTrip;
    uint32 ulOpHalfHours;       // run timer in 30 min intervals																						10ytes
    
    BYTE byEEPROMSave1;				// these are used to see if the EEPROM is good or not (2)                                                           11bytes
    BYTE byEEPROMSave2;             //                                                                                                                  12bytes
}structInfo2;



const struct __attribute__ ((__packed__))
{  
   uint16 eeprom_i2c_data; 
} structeeprom;


typedef struct __attribute__ ((__packed__)) {
	uint8_t RDMUID[6];
	uint16_t ADCBuffer;
	
    float flTemp_Scalar;
	uint32_t uiTempFaultCount;
    uint16_t uiFanDutyCycle;
}RemoteData;

uint32 uiTempFaultCount;
bool isInTempFault;
bool IsSlave;


/*****************************************************************************/
/* Macro Functions:                                                          */
/*****************************************************************************/
#define LengthOf(Array) (sizeof(Array)/sizeof(Array[0]))

/* Scalable Array Initializer */
#define newArray(ArraySize, InitialValue) \
	{ [0 ... (ArraySize)-1] = (InitialValue) }

/* for loop shortcut */
#define loopSet(x,y,z) \
	{for (size_t i=0;i<(z);i++)(x)[i] = (y);}

#endif // End of _main_h_
