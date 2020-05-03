/*******************************************************************************
* File Name: ADC_INT.c
* Version 3.10
*
*  Description:
*    This file contains the code that operates during the ADC_SAR interrupt
*    service routine.
*
*   Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "ADC.h"
#include "cyapicallbacks.h"


/******************************************************************************
* Custom Declarations and Variables
* - add user inlcude files, prototypes and variables between the following
*   #START and #END tags
******************************************************************************/
/* `#START ADC_SYS_VAR`  */

#include "device.h"
#include "main.h" 

extern uint16 uiADCAverage_TEMP[COUNT];
extern uint16 uiADCAverage_VSENSE[COUNT];
extern uint16 uiADCAverage_EXTTEMP[COUNT];
uint16 (*SampleArrayPtr[NUMADCCHANNELS]) = { uiADCAverage_TEMP, uiADCAverage_VSENSE, uiADCAverage_EXTTEMP};

/* `#END`  */

#if(ADC_IRQ_REMOVE == 0u)


    /******************************************************************************
    * Function Name: ADC_ISR
    *******************************************************************************
    *
    * Summary:
    *  Handle Interrupt Service Routine.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  No.
    *
    ******************************************************************************/
    CY_ISR( ADC_ISR )
    {
        #ifdef ADC_ISR_INTERRUPT_CALLBACK
            ADC_ISR_InterruptCallback();
        #endif /* ADC_ISR_INTERRUPT_CALLBACK */          

        
        /************************************************************************
        *  Custom Code
        *  - add user ISR code between the following #START and #END tags
        *************************************************************************/
          /* `#START MAIN_ADC_ISR`  */

        // start off ready to go to 0, 5 is too large
        static uint8 byChannel = 5;
        static uint8 byPointer[NUMADCCHANNELS] = {0};
        
        // get the last value grabbed by ADC
        SampleArrayPtr[byChannel][byPointer[byChannel]] = CY_GET_REG16(ADC_SAR_WRK0_PTR);
        ADC_Stop();

        // end of array?
        byPointer[byChannel]++;
        if(byPointer[byChannel] > COUNT)
            byPointer[byChannel] = 0;
        
        if(++byChannel >= NUMADCCHANNELS)
            byChannel = 0;
        AMux_Select(byChannel);
        ADC_Start();
        ADC_IRQ_Enable();
        ADC_StartConvert();
        
          /* `#END`  */
    }

#endif   /* End ADC_IRQ_REMOVE */

/* [] END OF FILE */
