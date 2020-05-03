/*

    pids.c
    *****************************************************************************
    * Copyright (2019 - 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
    

#include <stdio.h>
#include "main.h"
#include "pids.h"
#include "rdm.h"
#include "queue.h"
#include "rdmESTA.h"


extern structDeviceInfo DeviceInfo;
extern structRDM RDM;
extern structInfo Info;

extern bool blSaveToInfo;
extern bool blSaveToRDM;

bool getQueuedMessage(structRDMPacket *, PACKET_DEST);

bool getDMXPersonality(structRDMPacket *, PACKET_DEST);
bool setDMXPersonality(structRDMPacket *, PACKET_DEST);
bool getDMXPersonalityDescription(structRDMPacket *, PACKET_DEST);

bool getDeviceLabel(structRDMPacket *, PACKET_DEST);
bool setDeviceLabel(structRDMPacket *, PACKET_DEST);

bool getDeviceModelDescription(structRDMPacket *, PACKET_DEST);

bool getManufacturerLabel(structRDMPacket *, PACKET_DEST);

bool getDMXStartAddress(structRDMPacket *, PACKET_DEST);
bool setDMXStartAddress(structRDMPacket *, PACKET_DEST);

bool setSetUID(structRDMPacket *, PACKET_DEST);

/* ADD PROTOTYPE FOR CUSTOM PID REQUEST HANDLER
bool getCustomPid(structRDMPacket *, PACKET_DEST);
*/

const structRDMParameter RDMPIDStore[] =
{
	{
		.uiParameterID = E120_QUEUED_MESSAGE,
		.byGetLength = 1,
		.bySetLength = 0,
		.getRequestHandler = getQueuedMessage,
		.setRequestHandler = NULL,
	},
    {
		.uiParameterID = E120_DMX_PERSONALITY,
		.byGetLength = 0,
		.bySetLength = 1,
		.getRequestHandler = getDMXPersonality,
		.setRequestHandler = setDMXPersonality,
	},
	{
		.uiParameterID = E120_DMX_PERSONALITY_DESCRIPTION,
		.byGetLength = 1,
		.bySetLength = 0,
		.getRequestHandler = getDMXPersonalityDescription,
		.setRequestHandler = NULL,
	},
	{
		.uiParameterID = E120_DEVICE_LABEL,
		.byGetLength = 0,
		.bySetLength = 33,
		.getRequestHandler = getDeviceLabel,
		.setRequestHandler = setDeviceLabel,
		.blIsText = true,
	},
	{
		.uiParameterID = E120_DEVICE_MODEL_DESCRIPTION,
		.byGetLength = 0,
		.bySetLength = 0,
		.getRequestHandler = getDeviceModelDescription,
		.setRequestHandler = NULL,
		.blIsText = true,
	},
	{
		.uiParameterID = E120_MANUFACTURER_LABEL,
		.byGetLength = 0,
		.bySetLength = 0,
		.getRequestHandler = getManufacturerLabel,
		.setRequestHandler = NULL,
		.blIsText = true,
	},
	{
		.uiParameterID = E120_DMX_START_ADDRESS,
		.byGetLength = 0,
		.bySetLength = 2,
		.getRequestHandler = getDMXStartAddress,
		.setRequestHandler = setDMXStartAddress,
		.blIsHidden = true,
	},
	{
    	.uiParameterID = _RDM_UID,
    	.byGetLength = 0,
    	.bySetLength = 4,
    	.blIsHidden = true,
    	.getRequestHandler = NULL,
    	.setRequestHandler = setSetUID,
    },
    
	/* ADD DEFINITION OF CUSTOM PID
	Required: Required to function correctly.
	Manufacturer: Required by SUPPORTED_PARAMETERS. Used for manufacturer defined PIDs.
	{
		.uiParameterID = NED_CUSTOM_PID,			Required		PID defined in pid.h.
		.strDesc = "Dummy PID",						Manufacturer	PID description (Max 32 chars).												
		.ulMinValue = 0,							Manufacturer	Minimum return value.
		.ulMaxValue = 1,							Manufacturer	Maximum return value.
		.ulDefValue = 0,							Manufacturer	Default return value.
		.byDataType = 0,							Manufacturer	Data type (defined in RDM standard SUPPORTED_PARAMETERS).
		.byUnits = 0,								Manufacturer	Units (defined in RDM standard SUPPORTED_PARAMETERS).
		.byPrefix = 0,								Manufacturer	Prefix (defined in RDM standard SUPPORTED_PARAMETERS).
		.byGetLength = 0,							Required		Expected length of GET request.
		.bySetLength = 4,							Required		Expected length of SET request.
		.getRequestHandler = getCustomPid,			Required		Function pointer to GET request handler. Set to NULL if not supported.
		.setRequestHandler = NULL,					Required		Function pointer to SET request handler. Set to NULL if not supported.
	},
	*/
};
const uint8 RDMPIDStoreLength = sizeof(RDMPIDStore)/sizeof(RDMPIDStore[0]);

structDMXPersonality dmxPtys[] =
{
	{
		.uiSlotCount = 6,
		.cDesc = "6ch 8Bit",
	},
	{
		.uiSlotCount = 12,
		.cDesc = "6ch 16Bit",
	},
   	{
		.uiSlotCount = 1,
		.cDesc = "1ch 8Bit",
	},
   	{
		.uiSlotCount = 2,
		.cDesc = "1ch 16Bit",
	}
};

bool getQueuedMessage(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
   	if(Dest) {} //To prevent warning. Will compile to NO-OP.
	if(!queue_count())
	{
		RDMPacket->uiParameterID = CYSWAP_ENDIAN16(E120_STATUS_MESSAGES);
		RDMPacket->byParameterLength = 0;
	}
	else
	{
		const structQueueItem QI = queue_front();
		RDMPacket->uiParameterID = QI.uiParameterID;
		RDMPacket->byCommandClass = QI.byCommandClass;
		memcpy(RDMPacket->byParameterData, QI.byParameterData, QI.byParameterLength);
		RDMPacket->byParameterLength = QI.byParameterLength;
	}
	return true;
}

bool getDMXPersonality(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	if(Dest) {} //To prevent warning. Will compile to NO-OP.
	RDMPacket->byParameterData[0] = RDM.stRDMPersonality;
	RDMPacket->byParameterData[1] = _RDM_PERSONALITY_COUNT - 1;
	RDMPacket->byParameterLength = 2;
	return true;
}

bool setDMXPersonality(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	if(Dest == DEVICE)
	{
		uint8 Personality = RDMPacket->byParameterData[0] - 1;
		if(Personality < _RDM_PERSONALITY_COUNT - 1)
		{
			RDM.stRDMPersonality = Personality + 1;
			DeviceInfo.uiDMXFootprint = CYSWAP_ENDIAN16(dmxPtys[Personality].uiSlotCount);
			DeviceInfo.uiDMXPersonality = CYSWAP_ENDIAN16(UINT8_TO_UINT16(Personality + 1, _RDM_PERSONALITY_COUNT - 1));
	        
			RDMPacket->byParameterLength = 0;
			blSaveToInfo = true;
			blSaveToRDM = true;
			
			RDM_RespondTimer(RDMPacket, 50);
			queue_enqueue(E120_DMX_PERSONALITY, E120_SET_COMMAND, NULL, 0);
		}
		else
		{
			RDM_RespondNACK(RDMPacket, E120_NR_DATA_OUT_OF_RANGE);
		}
	}
	return false;
}

bool getDMXPersonalityDescription(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	uint8 Personality = RDMPacket->byParameterData[0];
	if(Personality >= 1 && Personality <= sizeof(dmxPtys)/sizeof(dmxPtys[0]))
	{
		RDMPacket->byParameterData[1] = HI8(dmxPtys[Personality-1].uiSlotCount);
		RDMPacket->byParameterData[2] = LO8(dmxPtys[Personality-1].uiSlotCount);
		memcpy(&RDMPacket->byParameterData[3], dmxPtys[Personality-1].cDesc, strlen(dmxPtys[Personality-1].cDesc));
		RDMPacket->byParameterLength = 3+strlen(dmxPtys[Personality-1].cDesc);
		return true;
	}
	else if(Dest == DEVICE)
	{
		RDM_RespondNACK(RDMPacket, E120_NR_DATA_OUT_OF_RANGE);
		return false;
	}
	return false;
}

bool getDeviceLabel(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	if(Dest) {} //To prevent warning. Will compile to NO-OP.
    memcpy(RDMPacket->byParameterData, RDM.stRDMIdentity, strlen((char *)RDM.stRDMIdentity));
	RDMPacket->byParameterLength = strlen((char *)RDM.stRDMIdentity);
	return true;
}

bool setDeviceLabel(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	if(Dest == DEVICE)
	{
        memset(RDM.stRDMIdentity, 0, sizeof(RDM.stRDMIdentity));
    	memcpy(RDM.stRDMIdentity, RDMPacket->byParameterData, RDMPacket->byParameterLength);
    	RDMPacket->byParameterLength = 0;
    	blSaveToRDM = true;
        
        RDM_RespondTimer(RDMPacket, 50);
		queue_enqueue(E120_DEVICE_LABEL, E120_SET_COMMAND, NULL, 0);
	}
	return false;
}

bool getDeviceModelDescription(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	if(Dest) {} //To prevent warning. Will compile to NO-OP.
    memcpy(RDMPacket->byParameterData, RDM.stRDMModel, strlen((char *)RDM.stRDMModel));
	RDMPacket->byParameterLength = strlen((char *)RDM.stRDMModel);
	return true;
}

bool getManufacturerLabel(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	if(Dest) {} //To prevent warning. Will compile to NO-OP.
    memcpy(RDMPacket->byParameterData, RDM.stRDMManufacturer, strlen((char *)RDM.stRDMManufacturer));
	RDMPacket->byParameterLength = strlen((char *)RDM.stRDMManufacturer);
	return true;
}

bool getDMXStartAddress(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	if(Dest) {} //To prevent warning. Will compile to NO-OP.
	
    RDMPacket->byParameterData[0] = HI8(Info.uiDMXChannel[0]);
	RDMPacket->byParameterData[1] = LO8(Info.uiDMXChannel[0]);
	RDMPacket->byParameterLength = 2;
	return true;
}

bool setDMXStartAddress(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	uint16 dmxAddress = UINT8_TO_UINT16(RDMPacket->byParameterData[0], RDMPacket->byParameterData[1]);
	if(dmxAddress >= 1 && dmxAddress <= 512)
	{
		DeviceInfo.uiDMXStartAddress = CYSWAP_ENDIAN16(dmxAddress);
		RDMPacket->byParameterLength = 0;
		blSaveToInfo = true;
  		blSaveToRDM = true;

		if(Dest == DEVICE)
		{
            RDM_RespondTimer(RDMPacket, 50);
			queue_enqueue(E120_DMX_START_ADDRESS, E120_SET_COMMAND, NULL, 0);
		}
		return false;
	}
	else if(Dest == DEVICE)
	{
		RDM_RespondNACK(RDMPacket, E120_NR_DATA_OUT_OF_RANGE);
		return false;
	}
	return false;
}

extern bool blRDMRunCalibration;
bool getPerformSelftest(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	if(Dest == DEVICE)
	{
		RDMPacket->byParameterData[0] = blRDMRunCalibration;
		RDMPacket->byParameterLength = 1;
		return true;
	}
	else
		return false;
}

bool setSetUID(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	if(RDMPacket->byParameterLength == 4)
	{
		uint8 UID[4] = {0};
		memcpy(UID, RDMPacket->byParameterData, 4);
		
		if(Dest == DEVICE)
		{
			RDMPacket->byResponseType = E120_RESPONSE_TYPE_ACK;
			RDMPacket->byParameterLength = 0;
			RDM_RespondMsg(RDMPacket);
		}
		
//		memcpy(&RDM.RDM_DevID[2], UID, 4);
        sprintf((char *)RDM.stRDMIdentity, _RDM_IDENTITY);
		blSaveToRDM = true;
	}
	else if(Dest == DEVICE)
		RDM_RespondNACK(RDMPacket, E120_NR_FORMAT_ERROR);
		
	return false;
}



/* ADD IMPLEMENTATION FOR REQUEST HANDLER
bool getCustomPid(structRDMPacket *RDMPacket, PACKET_DEST Dest)
{
	RDMPacket->byParameterData[0] = 0;					RDM->byParameterData[] contains the response data.
	RDMPacket->byParameterLength = 1;					RDM->byParameterLength contains the length of response data.
	return true;										Return true to respond normally, false if you call RDM_RespondNACK or RDM_RespondTimer.
}
*/
/* [] END OF FILE */
