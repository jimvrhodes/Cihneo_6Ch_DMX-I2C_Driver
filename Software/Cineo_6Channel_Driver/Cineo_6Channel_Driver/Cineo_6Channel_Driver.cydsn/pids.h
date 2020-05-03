/*

    pids.h
    *****************************************************************************
    * Copyright (2019 - 2020) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
    
#ifndef __PIDS_H
#define __PIDS_H

#include "cytypes.h"
    
#define DMX_START_ADDRESS Info.uiDMXChannel

#define _RDM_PERSONALITY_COUNT	5   // this should be 4, but it breaks the set method
#define _RDM_IDENTITY			"Cineo 6Chan"
#define _RDM_UID				0xE100
#define _RDM_MANUFACTURER		"Cineo"
#define _RDM_MODEL				"C6Chan"

typedef struct
{
	uint16 uiSlotCount;
	char *cDesc;
} structDMXPersonality;

#endif /* __PIDS_H */
/* [] END OF FILE */
