/*

    queue.h
    *****************************************************************************
    * Copyright (2014 - 2019) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
    
#ifndef __QUEUE_H
#define __QUEUE_H

#include "cytypes.h"
#include <stdbool.h>
#include <inttypes.h>
    
#define QUEUE_LENGTH 10
#define QUEUE_MEMORY 20
    
typedef struct __attribute__((__packed__)) {
    uint16 uiParameterID;
    uint8 byCommandClass;
    uint8 byParameterLength;
    uint8* byParameterData;
} structQueueItem;

bool queue_enqueue(uint16, uint8, uint8*, uint8);
void queue_dequeue(void);
structQueueItem queue_front(void);
uint8 queue_count(void);

#endif
/* [] END OF FILE */
