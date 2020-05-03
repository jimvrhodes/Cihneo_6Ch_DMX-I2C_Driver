/*
    main.h
    *****************************************************************************
    * Copyright (2019-2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
*/

#ifndef _main_h_
#define _main_h_
#include <project.h>
#include "cytypes.h"
#include <stdbool.h>
//    
// 
// Bootloader
#define CY_BOOTLOADABLE_Bootloadable_H
//
typedef uint16 UINT;
typedef uint8 BYTE;
typedef union
{
   UINT u16;
   struct
   {
        BYTE u8l;
        BYTE u8h;
   };

} uint16or8;
//
// release version
#define VERSIONMAJOR (BYTE) 0x01
#define VERSIONMINOR (UINT) 0x0000
#define VERSIONBUILD (BYTE) 0x00
//
// PROTOTYPES
// in subs.c
uint8 WriteToMaster (uint8 byCount);
void ReadFromMaster(void);
void GetDataFromDriver();
void FlushVCOMBuffer(void);
void CleanupVCOMBuffer(void);
void WriteLineUSBUART (BYTE* buffer, UINT count);    
void StartupLEDStrobe(BYTE byCount);
uint8 ConvertASCIIToHEX(char* ch);
BYTE ConvertHEXToASCII (BYTE value);
// Info eeprom items
void SaveInfoStruct(void);
void LoadInfoStruct(bool bRebuild);
// low level eeprom stuff
void LoadFromEEPROM(const uint16 EEPROMOffset, uint8 *restrict Data, const uint16 DataCount);
void SaveToEEPROM(const uint16 EEPROMOffset, uint8 *restrict Data, uint16 DataCount);
//
// in isr.c    
CY_ISR_PROTO(fnTIMER_ISR);
CY_ISR_PROTO(ADCIRQHandler);
#define NUMADCHANNELS 3
//
// in cli.c
void ManageConfigData(void);
bool CheckChecksum (void);
BYTE GenerateChecksum(void);
void SendACK(BYTE byAckCode, bool bForce);
void WriteLineVCOM (uint8_t* buffer, uint16_t count);
uint8 ConvertASCIIToHEX(char* ch);
uint8 ConvertToDecimal(char data);
void GenerateReadResponse(void);
//
// incoming and outgoing VCOM buffer size
#define VCOM_INPUT_BUFFER_SIZE 525
#define VCOM_OUTPUT_BUFFER_SIZE 70
//
#define VCOMBYTES 512
//
// the INT divider for faster handling
#define DIVIDER 50
//
// time we allow for the VCOM packet to arrive, or we call it bad and reset
// 5uSec increments
#define RECEPTIONTIME 10 * DIVIDER
#define TRANSMITTIME 5 * DIVIDER
//
#define VCOMSB 0x00
#define MAXUSBUARTBYTES 64
//
#define STARTUPLEDCYCLES 1
#define STARTUPLEDTIMER 150
//
// 1ms timer rate timer define, that clocks at a 1ms rate
#define FIVEHUNDREDMS 500		// this many TIMER8 ints
#define ONESEC 1000		// this many TIMER8 ints
#define HUNDREDMS 100	// we get to the SystemTimer routine every 10ms
//
//
// this is the EEPROM save structure
//
typedef struct __attribute__ ((__packed__))
{
    bool bACKOnOff;
    bool bAutoSaveOnOff;
    BYTE byPowerOnDefault;
	//
    UINT uiAuditNumber;
    //
    BYTE byEEPROMSave1;				// these are used to see if the EEPROM is good or not
    BYTE byEEPROMSave2;
} structInfo;
//
//
#define DATASAVE1 0x55
#define DATASAVE2 0xAA
//
// this is for EEPROM writing
#define CLEAR 0
#define SAVEIT 1
//
#define INFOBASE 0x00
//
// VCOM commands
enum CONSOLECOMMANDS
{
    PING = 0x10,    			        // ping back to show slave is alive
    ACK = 0x11,					        // this receives and sends a pattern to the GUI
    AUTOSAVE = 0x12,                    // save after each write, or wait for save command 0 = no, 1 = save, 2 = save now
    
    SLIDERCH1 = 0x20,
    SLIDERCH2 = 0x21,
    SLIDERCH3 = 0x22,
    SLIDERCH4 = 0x23,
    SLIDERCH5 = 0x24,
    SLIDERCH6 = 0x25,
    
    TEMP = 0x2A,
    VOLTS = 0x2B,
    READDRVVERSION = 0x2C,
    SETI2CADDRESS = 0x2D,
    
    READCHUBVERSION = 0x30,
    SENDTEMPERATURE = 0x31,
    SETAUDITCODE = 0xAA,                    // 16-bit audit value for readback by host
    SET_UPGRADE_FIRMWARE = 0xAB,
    SOFT_RESET = 0xBB,
    GET_STRUCTURE = 0xCC                // this is for the entire caboodle of programming all data at one time
};
//
//
// I2C stuff
#define NUMCHANNELS 6
#define CHANNELBASE SLIDERCH1
#define SLAVEADDRESS 22
#define SLAVEADDRESS_MAX (SLAVEADDRESS + 15)
#define USERSAVE 0xA5
#define FORCECHECKSUM 0xCA
#define SETDEFAULTS 0xCC
#define I2CIDLE 0
#define EDITABLE 20
// for Reading from master, do we clear or not
enum
{
    e_DONTSHOWSTATUS,
    e_SHOWSTATUS,
    e_STATUSEND
} READMASTER;
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
// packet format for a single byte value, simple command
enum PACKETA
{
    A_COMMAND,
    A_LENGTH,
    A_AUDIT,
    A_READWRITE,
    A_ACK,
    A_DATA,
    A_CHECKSUM
};
// packet format for a single byte value, simple command with universe selector
enum PACKETB
{
    B_COMMAND,
    B_LENGTH,
    B_AUDIT,
    B_READWRITE,
    B_ACK,
    B_UNIVERSE,
    B_DATA,
    B_CHECKSUM
};
// packet format for a two byte value, simple command with universe selector
enum PACKETC
{
    C_COMMAND,
    C_LENGTH,
    C_AUDIT,
    C_READWRITE,
    C_ACK,
    C_UNIVERSE,
    C_DATAH,
    C_DATAL,
    C_CHECKSUM
};
// packet format for a single byte value, simple command with universe selector
enum PACKETD
{
    D_COMMAND,
    D_LENGTH,
    D_AUDIT,
    D_READWRITE,
    D_ACK,
    D_DATAH,
    D_DATAL,
    D_CHECKSUM
};
// packet format for 4 data values, no universe (RDM version)
enum PACKETE
{
    E_COMMAND,
    E_LENGTH,
    E_AUDIT,
    E_READWRITE,
    E_ACK,
    E_DATA1,
    E_DATA2,
    E_DATA3,
    E_DATA4,
    E_CHECKSUM
};
// packet format for strings, no universe (RDM version)
enum PACKETF
{
    F_COMMAND,
    F_LENGTH,
    F_AUDIT,
    F_READWRITE,
    F_ACK,
    F_DATA,
    F_CHECKSUM      // this location can be variable
};
//
// packet offsets to where the data is
#define A_PACKETLENGTH 7
#define B_PACKETLENGTH 8
#define C_PACKETLENGTH 9
#define D_PACKETLENGTH 8
#define E_PACKETLENGTH 10
#define FDATAOFFSET (F_DATA + 1)
//
//
enum READWRITE
{
    e_WRITE,
    e_READ
};
//
enum ACKSTATE
{
    e_ACKOFF,
    e_ACKON
};    
enum RESET
{
    e_SOFTRESET,
    e_LOADEFAULTS,
    e_FLASHRELOAD
};
//
enum POWERONSTATE
{
    e_POWERONDEFAULTS,
    e_POWERONFLASH
};
//
enum AUTOSAVESTATE
{
    e_AUTOSAVEOFF,
    e_AUTOSAVEON,
    e_AUTOSAVENOW
};
//
enum ADCS
{
    e_TEMP,
    e_VSENSE,
    e_USBPRESENT
};
//
// CLI response characters
#define PINGRESPONSE 0x55
#define ACKGOODRESPONSE 0xAA
#define ACKBADRESPONSE 0xBB
#define ACKRDMALREADYSELECTEDRESPONSE 0xBC
#define ACKBADCHECKSUMRESPONSE 0xBD
// command lengths in bytes
#define ACKLENGTH A_PACKETLENGTH
#define AUTOSAVELENGTH A_PACKETLENGTH
#define AUDITCODELENGTH B_PACKETLENGTH
#define RESETLENGTH A_PACKETLENGTH
#define BOOTLOADLENGTH B_PACKETLENGTH
#define READCHUBVERSIONLENGTH E_PACKETLENGTH
#define SENDTEMPERATURELENGTH D_PACKETLENGTH
#define SLIDERLENGTH D_PACKETLENGTH
#define TEMPLENGTH A_PACKETLENGTH
#define VOLTLENGTH A_PACKETLENGTH
#define READDRVVERSIONLENGTH E_PACKETLENGTH
#define SETI2CADDRESSLENGTH A_PACKETLENGTH
//
// these are the bootload "unlock" bytes to be certain that we wanted to do that!
#define BOOTLOADBYTEH 0x55
#define BOOTLOADBYTEL 0xAA
#endif // End of _main_h_
