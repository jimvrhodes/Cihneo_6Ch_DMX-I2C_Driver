/*

    dmx.c
    *****************************************************************************
    * Copyright (2014 - 2019) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
    
#include "RDM_Header.h"


// DMX timers for cable on/off glitches
extern uint16 uiDMXLossTimer;
extern uint16 uiDMXPresentTimer;
extern uint16 uiwDMXLossTimer;
extern uint16 uiwDMXPresentTimer;
//
extern structInfo Info;
extern structRDMPacket RDMResPacket;
extern structRDM RDM;

extern volatile uint8 byDMXGood;
extern volatile uint8 byDMXData[];
extern uint8 bySlaveUnitID;


//write a function to handle RDM response transmission
//note the optional break
void transmitResponse(const uint8 *data, uint8 length, uint8 isbreak)
{
    DMX_Control_Reg_Write(0x02);
        
	if(isbreak)
	{
      	DMX_Control_Reg_Write(0x03);
		CyDelayUs(125);
	}

	DMX_Control_Reg_Write(0x02);
	CyDelayUs(16);

	DMX_UART_PutArray(data, length);
	while (!(DMX_UART_TXSTATUS & DMX_UART_TX_STS_COMPLETE));
	CyDelayUs(8);

	DMX_Control_Reg_Write(0x00);
}


#define CHECK_FRAMES 4
#define DMX_BUFFER_LEN (NUMCHANNELS*2)

#define SC_DMX 0x00
#define SC_RDM 0xCC

// so that we can use the RDM routine for either port
void (*const PWM_ADIM_WriteCompare[NUMCHANNELS]) (uint16_t) = {PWM_ADIM_1_WriteCompare1, PWM_ADIM_1_WriteCompare2, PWM_ADIM_2_WriteCompare1, PWM_ADIM_2_WriteCompare2, 
                                                               PWM_BOTHDIM5_WriteCompare1, PWM_BOTHDIM6_WriteCompare1};

CY_ISR(DMXRxISRHandler)
{
	static uint8 data_buffer[DMX_BUFFER_LEN] = {};
	static size_t data_idx = 0;
	
	static enum {e_IDLE, e_START, e_DATA, e_RDM} state = e_IDLE;
	if (DMX_UART_RXSTATUS & DMX_UART_RX_STS_STOP_ERROR) {
		static uint16 check_frame[CHECK_FRAMES] = {};
		static uint8 check_idx = 0;
		
		check_frame[check_idx++] = data_idx;
		if (check_idx >= CHECK_FRAMES) check_idx = 0;
		
		bool check = true;
		uint16 check_val = check_frame[0];
		for (uint8 i=1; i<CHECK_FRAMES; i++) if (check_frame[i] != check_val)
			check = false;
		
		if (check) 
        {
			memcpy((uint8 *)byDMXData, data_buffer, (NUMCHANNELS*2));
		    byDMXGood = true;
		}
		
		data_idx = 0;
		state = e_START;
		DMX_UART_ClearRxBuffer();
		RDM_ResetState();
	} 
    else while (DMX_UART_RXSTATUS & DMX_UART_RX_STS_FIFO_NOTEMPTY) 
    {
		uint8 data = DMX_UART_RXDATA;
			
		switch (state)
        {
		    case e_IDLE:
            default:
            break;
			
		    case e_START:
    			if (data == SC_DMX)
                    state = e_DATA;
    			else if (data == SC_RDM)
                    state = e_RDM;
    			else
                    state = e_IDLE;
			break;
			
    		case e_DATA:
	    		for (uint8 i=0; i<NUMCHANNELS; i++)
                {
                    // 8 bit values
                    if(RDM.stRDMPersonality == 1 || RDM.stRDMPersonality == 3)
                    {
                        if (Info.uiDMXChannel[i] == data_idx)
                            data_buffer[i] = data;
                    }
                    // 16-bit values
                    else
                    {
                        if (Info.uiDMXChannel[i] == data_idx) 
                            data_buffer[i*2] = data;
                        else if (Info.uiDMXChannel[i] + 1 == data_idx) 
                            data_buffer[(i*2)+1] = data;
                    }
                }
			break;
			
    		case e_RDM:
	    		if (!IsSlave)
                    RDM_AcceptByte(data);
                
			    uiDMXLossTimer = DMXLOSSTIME;
			break;
		}
		
		data_idx++;
	}
}

