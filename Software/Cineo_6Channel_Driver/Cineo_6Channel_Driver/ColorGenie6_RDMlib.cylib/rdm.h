/*

    rdm.h
    *****************************************************************************
    * Copyright (2014 - 2019) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
    
#ifndef __RDM_H
#define __RDM_H

#include "cytypes.h"
#include <stdbool.h>
#include <inttypes.h>
    
#define PARAMETER_DATA_BUFFER_SIZE 231
#define UID_LENGTH 6
#define UINT8_TO_UINT16(x, y) (((uint16)(x) << 8) | ((uint8)(y)))

typedef struct __attribute__((__packed__)) {
    uint8 byStartCode;
    uint8 bySubStartCode;
    uint8 byMessageLength;
    uint8 byDestinationUID[UID_LENGTH];
    uint8 bySourceUID[UID_LENGTH];
    uint8 byTransactionNum;
    uint8 byResponseType;
    uint8 byMessageCount;
    uint16 uiSubDevice;
    uint8 byCommandClass;
    uint16 uiParameterID;
    uint8 byParameterLength;
    uint8 byParameterData[PARAMETER_DATA_BUFFER_SIZE];
} structRDMPacket;

typedef struct __attribute__((__packed__)) {
    uint16 uiRDMProtocol;
    uint16 uiDeviceModel;
    uint16 uiProductCategory;
    uint32 ulSoftwareVersion;
    uint16 uiDMXFootprint;
    uint16 uiDMXPersonality;
    uint16 uiDMXStartAddress;
    uint16 uiSubDeviceCount;
    uint8 bySensorCount;
} structDeviceInfo;

typedef struct __attribute__((__packed__)) {
    uint8 RDM_DevID[6]; // this is the UID parameters
    uint16 uiDMXStartAddress;
    uint8 stRDMPersonality;
    uint8 stRDMIdentity[33];
    uint8 stRDMManufacturer[33];
    uint8 stRDMModel[30];
    uint8 byLevel;
} structRDM;

typedef enum {
    IDLE,
    SUB_START_CODE,
    MESSAGE_LENGTH,
    DESTINATION_UID,
    SOURCE_UID,
    TRANSACTION_NUMBER,
    RESPONSE_TYPE,
    MESSAGE_COUNT,
    SUB_DEVICE,
    COMMAND_CLASS,
    PARAMETER_ID,
    PARAMETER_LENGTH,
    PARAMETER_DATA,
    CHECKSUM
} RDMSTATE;

typedef enum {
    UNKNOWN,
    DEVICE,
    BROADCAST,
    VENDORCAST
} PACKET_DEST;

typedef struct __attribute__((__packed__)) {
    uint8 byPreamble[8];
    uint8 byDeviceID[12];
    uint8 byChecksum[4];
} structDiscovery;

typedef struct __attribute__((__packed__)) {
    const uint16 uiParameterID;
    const char* strDesc;
    const uint32 ulMinValue;
    const uint32 ulMaxValue;
    const uint32 ulDefValue;
    const uint8 byDataType;
    const uint8 byUnits;
    const uint8 byPrefix;
    const uint8 byGetLength;
    const uint8 bySetLength;
    bool (*getRequestHandler)(structRDMPacket*, PACKET_DEST);
    bool (*setRequestHandler)(structRDMPacket*, PACKET_DEST);
    bool blIsHidden;
    bool blIsText;
} structRDMParameter;

void RDM_Init(uint8* deviceUID, structDeviceInfo *deviceInfo, uint16 dmxAddress, bool subDevice, void (*transmitResponse)(const uint8*, uint8, uint8));
void RDM_HandleRequest(structRDMPacket*, PACKET_DEST);
void RDM_DiscoveryResponse(void);
void RDM_RespondMsg(structRDMPacket*);
void RDM_RespondNACK(structRDMPacket*, uint16);
const structRDMParameter* RDM_LookupPID(uint16);
void RDM_RespondTimer(structRDMPacket*, uint16);
void RDM_ResetState(void);
void RDM_AcceptByte(uint8);

#endif /* __RDM_H */
/* [] END OF FILE */
