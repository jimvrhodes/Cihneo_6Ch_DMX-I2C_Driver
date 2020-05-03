/*

    RDM_Header.h
    *****************************************************************************
    * Copyright (2014 - 2019) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/

#ifndef _rdm_header_h_
#define _rdm_header_h_
        

#include "cytypes.h"
#include <stdbool.h>
#include <inttypes.h>
#include "rdm.h"
#include "dmx.h"
#include "pids.h"
#include "queue.h"
#include "rdmESTA.h"
#include "main.h"
    

extern void CyDelayUs(uint32);
extern void DMX_UART_PutArray(const uint8*,uint8);
extern void DMX_Control_Reg_Write(uint8);
//
reg8 DMX_UART_TXSTATUS;
reg8 DMX_UART_TX_STS_COMPLETE;
reg8 DMX_UART_RXSTATUS;
reg8 DMX_UART_RX_STS_STOP_ERROR;
reg8 DMX_UART_RX_STS_FIFO_NOTEMPTY;
reg8 DMX_UART_RXDATA;
//
extern void PWM_ADIM_1_WriteCompare1(uint16_t);
extern void PWM_ADIM_1_WriteCompare2(uint16_t);
extern void PWM_ADIM_2_WriteCompare1(uint16_t);
extern void PWM_ADIM_2_WriteCompare2(uint16_t);
extern void PWM_BOTHDIM5_WriteCompare1(uint16_t);
extern void PWM_BOTHDIM6_WriteCompare1(uint16_t);
typedef unsigned int size_t;
extern void DMX_UART_ClearRxBuffer(void);
//
extern bool blSaveToInfo;
extern bool blSaveToRDM;
//
void RDM_Init(uint8* deviceUID, structDeviceInfo *deviceInfo, uint16 dmxAddress, bool subDevice, void (*transmitResponse)(const uint8*, uint8, uint8));    
void transmitResponse(const uint8 *data, uint8 length, uint8 isbreak);
CY_ISR(DMXRxISRHandler);

#endif // End of _rdm_header_h_
