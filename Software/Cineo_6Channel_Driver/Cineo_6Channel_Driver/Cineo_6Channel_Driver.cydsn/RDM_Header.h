/*

    RDM_Header.h
    *****************************************************************************
    * Copyright (2014 - 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/

#ifndef _rdm_header_h_
#define _rdm_header_h_
        
#include "cytypes.h"
#include <stdbool.h>
#include <inttypes.h>
//
#include <rdm.h>
#include <queue.h>
#include <rdmESTA.h>
#include <pids.h>
#include <main.h>

    
    
structRDM RDM;
structDeviceInfo DeviceInfo = {
    .uiRDMProtocol = CYSWAP_ENDIAN16(E120_PROTOCOL_VERSION),
    .uiDeviceModel  = 0x0000,
    .uiProductCategory = 0,
    .ulSoftwareVersion = ((VERSIONMINOR << 24) | VERSIONMAJOR << 8),
    .uiDMXFootprint = 0x0100,
    .uiDMXPersonality = 0x0402,     // four available, 2 is the default
    .uiDMXStartAddress = 0x0001,
    .uiSubDeviceCount = 0,
    .bySensorCount = 0,
};

// VID
volatile uint8 deviceUID[6] = {VID1, VID2, 0x01, 0x02, 0x03, 0x04};
extern uint8 byRDM_Eval;
extern uint8 byRDM_Identify;
bool blSaveToRDM;
bool blSaveToInfo;
bool blSaveOnChange = false;

bool RDM_Identify;
structRDMPacket RDMResPacket;
RDMSTATE state = IDLE;
uint8 paramCount = 0;
uint16 calculatedChecksum = 0;
uint16 receivedChecksum = 0;
bool isDevice = true;
bool isBroadcast = true;
bool isVendorcast = true;
bool isSubDevice;
bool RDM_pendingDequeue = false;
bool RDM_Identify = false;

structQueueItem Queue[QUEUE_LENGTH];
uint8 QueueMemory[QUEUE_MEMORY] = { 0 };
uint8 QueueIndex = 0;
uint8* QueueMemPtr = QueueMemory;
uint8 QueueMemLen = QUEUE_MEMORY;

bool blSaveToInfo;
bool blSaveToRDM;

extern void RDM_Init(uint8* deviceUID, structDeviceInfo *deviceInfo, uint16 dmxAddress, bool subDevice, void (*transmitResponse)(const uint8*, uint8, uint8));    
extern void transmitResponse(const uint8 *data, uint8 length, uint8 isbreak);
extern CY_ISR(DMXRxISRHandler);
void (*TRANSMIT_RESPONSE)(const uint8*, uint8, uint8);
structDeviceInfo *DEVICEINFO;

#endif // End of _rdm_header_h_
