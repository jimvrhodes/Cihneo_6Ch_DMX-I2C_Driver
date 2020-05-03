/*

    rdm.c
    *****************************************************************************
    * Copyright (2019 - 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
    

#include <stdio.h>
#include <stdbool.h>
#include "rdm.h"
#include "rdmESTA.h"
#include "pids.h"
#include "queue.h"
#include "main.h"

extern structRDMPacket RDMResPacket;
extern RDMSTATE state;
extern uint8 paramCount;
extern uint16 calculatedChecksum;
extern uint16 receivedChecksum;
extern bool isDevice;
extern bool isBroadcast;
extern bool isVendorcast;

extern const structRDMParameter RDMPIDStore[];
extern const uint8 RDMPIDStoreLength;
extern bool isSubDevice;

extern bool RDM_pendingDequeue;
extern bool RDM_Identify;

extern void (*TRANSMIT_RESPONSE)(const uint8*, uint8, uint8);

extern structRDM RDM;
extern structDeviceInfo *DEVICEINFO;

extern uint16 dmxStartAddress;

void RDM_Init(uint8* deviceUID, structDeviceInfo *deviceInfo, uint16 dmxAddress, bool subDevice, void (*transmitResponse)(const uint8*, uint8, uint8))
{
    memcpy(RDM.RDM_DevID, deviceUID, UID_LENGTH);
	DEVICEINFO = deviceInfo;
    if (!dmxAddress)
        dmxAddress = 0xFFFF;
    DEVICEINFO->uiDMXStartAddress = CYSWAP_ENDIAN16(dmxAddress);
    DEVICEINFO->uiDMXPersonality = (uint16)RDM.stRDMPersonality + ((_RDM_PERSONALITY_COUNT - 1) << 8);
    isSubDevice = subDevice;
    TRANSMIT_RESPONSE = transmitResponse;
}

void RDM_HandleRequest(structRDMPacket* RDMPacket, PACKET_DEST PacketDest)
{
    static uint8 discMuteFlag = 0;
    bool respondedFlag = false;

    if (!isSubDevice) {
        if (CYSWAP_ENDIAN16(RDMPacket->uiSubDevice) > 1) {
            RDM_RespondNACK(RDMPacket, E120_NR_SUB_DEVICE_OUT_OF_RANGE);
            return;
        }
        switch (CYSWAP_ENDIAN16(RDMPacket->uiParameterID)) {
        case E120_DISC_UN_MUTE:
            if (!RDMPacket->byParameterLength) {
                discMuteFlag = 0;
                memset(RDMPacket->byParameterData, 0, 2);
                RDMPacket->byParameterLength = 2;
                respondedFlag = true;
            } else
                return;
            break;

        case E120_DISC_MUTE:
            if (!RDMPacket->byParameterLength) {
                discMuteFlag = 1;
                memset(RDMPacket->byParameterData, 0, 2);
                RDMPacket->byParameterLength = 2;
                respondedFlag = true;
            } else
                return;
            break;

        case E120_DISC_UNIQUE_BRANCH:
            if (!discMuteFlag)
                if (memcmp(RDM.RDM_DevID, RDMPacket->byParameterData, UID_LENGTH) >= 0
                    && memcmp(RDM.RDM_DevID, &RDMPacket->byParameterData[6], UID_LENGTH) <= 0) {
                    RDM_DiscoveryResponse();
                    return;
                }
            break;

        case E120_DEVICE_INFO:
            if (RDMPacket->byCommandClass == E120_GET_COMMAND && !RDMPacket->byParameterLength) {
                memcpy(RDMPacket->byParameterData, DEVICEINFO, sizeof(structDeviceInfo));
                RDMPacket->byParameterLength = sizeof(structDeviceInfo);
                respondedFlag = true;
            } else if (RDMPacket->byCommandClass == E120_SET_COMMAND) {
                RDM_RespondNACK(RDMPacket, E120_NR_UNSUPPORTED_COMMAND_CLASS);
                return;
            } else {
                RDM_RespondNACK(RDMPacket, E120_NR_FORMAT_ERROR);
                return;
            }
            break;

        case E120_IDENTIFY_DEVICE:
            if (RDMPacket->byCommandClass == E120_GET_COMMAND && !RDMPacket->byParameterLength) {
                RDMPacket->byParameterData[0] = RDM_Identify;
                RDMPacket->byParameterLength = 1;
                respondedFlag = true;
            } else if (RDMPacket->byCommandClass == E120_SET_COMMAND && RDMPacket->byParameterLength == 1) {
                if (RDMPacket->byParameterData[0] >= 0 && RDMPacket->byParameterData[0] <= 1) {
                    RDM_Identify = RDMPacket->byParameterData[0];
                    RDMPacket->byParameterLength = 0;
                    respondedFlag = true;
                } else {
                    RDM_RespondNACK(RDMPacket, E120_NR_DATA_OUT_OF_RANGE);
                    return;
                }
            } else {
                RDM_RespondNACK(RDMPacket, E120_NR_FORMAT_ERROR);
                return;
            }
            break;

        case E120_SOFTWARE_VERSION_LABEL:
            if (RDMPacket->byCommandClass == E120_GET_COMMAND && !RDMPacket->byParameterLength) {
                char softLabel[33];

                sprintf(softLabel, "v%d.%02d build %d", VERSIONMAJOR, VERSIONMINOR, VERSIONBUILD);

                strcpy((char*)RDMPacket->byParameterData, softLabel);
                RDMPacket->byParameterLength = strlen(softLabel);
                respondedFlag = true;
            } else if (RDMPacket->byCommandClass == E120_SET_COMMAND) {
                RDM_RespondNACK(RDMPacket, E120_NR_UNSUPPORTED_COMMAND_CLASS);
                return;
            } else {
                RDM_RespondNACK(RDMPacket, E120_NR_FORMAT_ERROR);
                return;
            }
            break;
        }
    } else if (CYSWAP_ENDIAN16(RDMPacket->uiSubDevice) != 1) {
        switch (CYSWAP_ENDIAN16(RDMPacket->uiParameterID)) {
        //Add PIDs to ignore here. Subdevice operation only.

        default:
            return;
            break;
        }
    }

    if (!respondedFlag) {
        switch (CYSWAP_ENDIAN16(RDMPacket->uiParameterID)) {
        case E120_SUPPORTED_PARAMETERS:
            if (RDMPacket->byCommandClass == E120_GET_COMMAND && !RDMPacket->byParameterLength) {
                uint8 i, j;
                for (i = 0, j = 0; i < RDMPIDStoreLength; i++, j += 2) {
                    if (!RDMPIDStore[i].blIsHidden) {
                        RDMPacket->byParameterData[j + 0] = HI8(RDMPIDStore[i].uiParameterID);
                        RDMPacket->byParameterData[j + 1] = LO8(RDMPIDStore[i].uiParameterID);
                    } else
                        j -= 2;
                }
                RDMPacket->byParameterLength = j;
            } else if (RDMPacket->byCommandClass == E120_SET_COMMAND) {
                RDM_RespondNACK(RDMPacket, E120_NR_UNSUPPORTED_COMMAND_CLASS);
                return;
            } else {
                RDM_RespondNACK(RDMPacket, E120_NR_FORMAT_ERROR);
                return;
            }
            break;

        case E120_PARAMETER_DESCRIPTION: {
            uint8 i = 0;
            bool hasCustomPID = false;

            for (i = 0; i < RDMPIDStoreLength; i++)
                if (RDMPIDStore[i].uiParameterID >= 0x8000 && !RDMPIDStore[i].blIsHidden)
                    hasCustomPID = true;

            if (!hasCustomPID) {
                RDM_RespondNACK(RDMPacket, E120_NR_UNKNOWN_PID);
                return;
            }
        }
            if (RDMPacket->byCommandClass == E120_GET_COMMAND && RDMPacket->byParameterLength == 2) {
                const uint16 PID = UINT8_TO_UINT16(RDMPacket->byParameterData[0], RDMPacket->byParameterData[1]);
                const structRDMParameter* RDMParameter = RDM_LookupPID(PID);

                if (RDMParameter != NULL && PID >= 0x8000) {
                    uint8 descLen = strlen(RDMParameter->strDesc);
                    RDMPacket->byParameterData[2] = (RDMParameter->byGetLength > RDMParameter->bySetLength ? RDMParameter->byGetLength : RDMParameter->bySetLength);
                    RDMPacket->byParameterData[3] = RDMParameter->byDataType;
                    RDMPacket->byParameterData[4] = (RDMParameter->getRequestHandler ? E120_CC_GET : 0)
                        | (RDMParameter->setRequestHandler ? E120_CC_SET : 0);
                    RDMPacket->byParameterData[5] = 0;
                    RDMPacket->byParameterData[6] = RDMParameter->byUnits;
                    RDMPacket->byParameterData[7] = RDMParameter->byPrefix;
                    memcpy(&RDMPacket->byParameterData[8], &RDMParameter->ulMinValue, sizeof(uint32));
                    memcpy(&RDMPacket->byParameterData[12], &RDMParameter->ulMaxValue, sizeof(uint32));
                    memcpy(&RDMPacket->byParameterData[16], &RDMParameter->ulDefValue, sizeof(uint32));
                    memcpy(&RDMPacket->byParameterData[20], RDMParameter->strDesc, descLen);
                    RDMPacket->byParameterLength = 20 + descLen;
                } else {
                    RDM_RespondNACK(RDMPacket, E120_NR_DATA_OUT_OF_RANGE);
                    return;
                }
            } else if (RDMPacket->byCommandClass == E120_SET_COMMAND) {
                RDM_RespondNACK(RDMPacket, E120_NR_UNSUPPORTED_COMMAND_CLASS);
                return;
            } else {
                RDM_RespondNACK(RDMPacket, E120_NR_FORMAT_ERROR);
                return;
            }
            break;

        default: {
            const structRDMParameter* RDMParameter = RDM_LookupPID(CYSWAP_ENDIAN16(RDMPacket->uiParameterID));
            if (RDMParameter != NULL) {
                if (RDMPacket->byCommandClass == E120_GET_COMMAND) {
                    if (RDMParameter->getRequestHandler != NULL)
                        if (RDMParameter->byGetLength == RDMPacket->byParameterLength) {
                            if (!RDMParameter->getRequestHandler(RDMPacket, PacketDest))
                                return;
                        } else {
                            RDM_RespondNACK(RDMPacket, E120_NR_FORMAT_ERROR);
                            return;
                        }
                    else {
                        RDM_RespondNACK(RDMPacket, E120_NR_UNSUPPORTED_COMMAND_CLASS);
                        return;
                    }
                } else if (RDMPacket->byCommandClass == E120_SET_COMMAND) {
                    if (RDMParameter->setRequestHandler != NULL)
                        if (RDMParameter->bySetLength == RDMPacket->byParameterLength
                            || (RDMParameter->blIsText && RDMPacket->byParameterLength <= RDMParameter->bySetLength)) {
                            if (!RDMParameter->setRequestHandler(RDMPacket, PacketDest))
                                return;
                        } else {
                            RDM_RespondNACK(RDMPacket, E120_NR_FORMAT_ERROR);
                            return;
                        }
                    else {
                        RDM_RespondNACK(RDMPacket, E120_NR_UNSUPPORTED_COMMAND_CLASS);
                        return;
                    }
                }
            } else {
                if (RDMPacket->byCommandClass != E120_DISCOVERY_COMMAND)
                    RDM_RespondNACK(RDMPacket, E120_NR_UNKNOWN_PID);
                return;
            }
        } break;
        }
    }
    if (PacketDest == DEVICE) {
        RDMPacket->byResponseType = E120_RESPONSE_TYPE_ACK;
        RDM_RespondMsg(RDMPacket);
        if (RDM_pendingDequeue && queue_count()) {
            queue_dequeue();
        }
    }
}

void RDM_DiscoveryResponse()
{
    structDiscovery RDMDiscovery = {
        .byPreamble = { 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xAA },
        .byDeviceID = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 },
        .byChecksum = { 0xAA, 0x55, 0xAA, 0x55 },
    };

    uint16 checksum = 0;
    uint8 i, j;
    for (i = 0, j = 0; i < 6; i++, j += 2) {
        RDMDiscovery.byDeviceID[j + 0] |= RDM.RDM_DevID[i];
        checksum += RDMDiscovery.byDeviceID[j + 0];

        RDMDiscovery.byDeviceID[j + 1] |= RDM.RDM_DevID[i];
        checksum += RDMDiscovery.byDeviceID[j + 1];
    }

    RDMDiscovery.byChecksum[0] |= HI8(checksum);
    RDMDiscovery.byChecksum[1] |= HI8(checksum);
    RDMDiscovery.byChecksum[2] |= LO8(checksum);
    RDMDiscovery.byChecksum[3] |= LO8(checksum);

    if (TRANSMIT_RESPONSE != NULL)
        TRANSMIT_RESPONSE((uint8*)&RDMDiscovery, sizeof(RDMDiscovery), false);
}

void RDM_RespondMsg(structRDMPacket* RDMPacket)
{
    uint16 checksum = 0;
    uint8* pItr = (uint8*)RDMPacket;
    uint8 i;

    RDMPacket->byStartCode = E120_SC_RDM;
    RDMPacket->byMessageLength = 24 + RDMPacket->byParameterLength;
    RDMPacket->byCommandClass++;

    for (i = 0; i < RDMPacket->byMessageLength; i++)
        checksum += *pItr++;

    *pItr++ = HI8(checksum);
    *pItr++ = LO8(checksum);

    if (TRANSMIT_RESPONSE != NULL)
        TRANSMIT_RESPONSE((uint8*)RDMPacket, RDMPacket->byMessageLength + 2, true);
}

void RDM_RespondNACK(structRDMPacket* RDMPacket, uint16 NACKCode)
{
    RDMPacket->byParameterData[0] = HI8(NACKCode);
    RDMPacket->byParameterData[1] = LO8(NACKCode);
    RDMPacket->byParameterLength = 2;
    RDMPacket->byResponseType = E120_RESPONSE_TYPE_NACK_REASON;
    RDM_RespondMsg(RDMPacket);
}

void RDM_RespondTimer(structRDMPacket* RDMPacket, uint16 timeAmount)
{
    RDMPacket->byParameterData[0] = HI8(timeAmount);
    RDMPacket->byParameterData[1] = LO8(timeAmount);
    RDMPacket->byParameterLength = 2;
    RDMPacket->byResponseType = E120_RESPONSE_TYPE_ACK_TIMER;
    RDM_RespondMsg(RDMPacket);
}

const structRDMParameter* RDM_LookupPID(uint16 PID)
{
    uint8 i;
    for (i = 0; i < RDMPIDStoreLength; i++)
        if (RDMPIDStore[i].uiParameterID == PID)
            return &RDMPIDStore[i];
    return NULL;
}

void RDM_ResetState()
{
    state = SUB_START_CODE;
    paramCount = 0;
    calculatedChecksum = 0;
    receivedChecksum = 0;
    isDevice = true;
    isBroadcast = true;
    isVendorcast = true;
    memset(&RDMResPacket, 0, sizeof(structRDMPacket));
}

void RDM_AcceptByte(uint8 dataByte)
{
    if (state >= SUB_START_CODE && state < CHECKSUM)
        calculatedChecksum += dataByte;
    switch (state) {
    case IDLE: {
    } break;

    case SUB_START_CODE: {
        if (dataByte == E120_SC_SUB_MESSAGE) {
            RDMResPacket.bySubStartCode = E120_SC_SUB_MESSAGE;
            state = MESSAGE_LENGTH;
        }
    } break;

    case MESSAGE_LENGTH: {
        if (dataByte >= 24) {
            RDMResPacket.byMessageLength = dataByte;
            state = DESTINATION_UID;
        } else
            state = IDLE;
    } break;

    case DESTINATION_UID: {
        if (dataByte != RDM.RDM_DevID[paramCount])
            isDevice = false;
        if (dataByte != 0xFF)
            isBroadcast = false;
        if (paramCount < 2 && dataByte != RDM.RDM_DevID[paramCount])
            isVendorcast = false;
        if (paramCount >= 2 && dataByte != 0xFF)
            isVendorcast = false;

        RDMResPacket.bySourceUID[paramCount] = dataByte;
        paramCount++;

        if (paramCount >= 6) {
            if (isDevice || isVendorcast || isBroadcast) {
                paramCount = 0;
                state = SOURCE_UID;
            } else
                state = IDLE;
        }
    } break;

    case SOURCE_UID: {
        RDMResPacket.byDestinationUID[paramCount++] = dataByte;
        if (paramCount >= 6) {
            paramCount = 0;
            state = TRANSACTION_NUMBER;
        }
    } break;

    case TRANSACTION_NUMBER: {
        RDMResPacket.byTransactionNum = dataByte;
        state = RESPONSE_TYPE;
    } break;

    case RESPONSE_TYPE: {
        RDMResPacket.byResponseType = dataByte;
        state = MESSAGE_COUNT;
    } break;

    case MESSAGE_COUNT: {
        if (!dataByte) {
            RDMResPacket.byMessageCount = dataByte;
            state = SUB_DEVICE;
        } else
            state = IDLE;
    } break;

    case SUB_DEVICE: {
        RDMResPacket.uiSubDevice = dataByte << (paramCount ? 8 : 0);
        paramCount++;
        if (paramCount >= 2) {
            paramCount = 0;
            state = COMMAND_CLASS;
        }
    } break;

    case COMMAND_CLASS: {
        RDMResPacket.byCommandClass = dataByte;
        state = PARAMETER_ID;
    } break;

    case PARAMETER_ID: {
        RDMResPacket.uiParameterID |= dataByte << (paramCount ? 8 : 0);
        paramCount++;
        if (paramCount >= 2) {
            paramCount = 0;
            state = PARAMETER_LENGTH;
        }
    } break;

    case PARAMETER_LENGTH: {
        if (dataByte == RDMResPacket.byMessageLength - 24) {
            RDMResPacket.byParameterLength = dataByte;
            if (dataByte)
                state = PARAMETER_DATA;
            else
                state = CHECKSUM;
        } else
            state = IDLE;
    } break;

    case PARAMETER_DATA: {
        RDMResPacket.byParameterData[paramCount++] = dataByte;
        if (paramCount >= RDMResPacket.byParameterLength) {
            paramCount = 0;
            state = CHECKSUM;
        }
    } break;

    case CHECKSUM: {
        receivedChecksum |= dataByte << (paramCount ? 0 : 8);
        paramCount++;
        if (paramCount >= 2) {
            if (receivedChecksum == calculatedChecksum + 0xCC) {
                PACKET_DEST PacketDest = UNKNOWN;

                if (isDevice)
                    PacketDest = DEVICE;
                if (isBroadcast)
                    PacketDest = BROADCAST;
                if (isVendorcast)
                    PacketDest = VENDORCAST;

                RDM_HandleRequest(&RDMResPacket, PacketDest);
            } else {
                CY_NOP;
            }
            state = IDLE;
        }
    } break;
    }
}
/* [] END OF FILE */
