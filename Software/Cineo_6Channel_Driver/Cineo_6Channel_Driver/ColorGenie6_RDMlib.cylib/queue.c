/*

    queue.c
    *****************************************************************************
    * Copyright (2014 - 2019) The NED Group
    ******************************************************************************
    see main.c for full disclaimer
    
*/
    

#include <stdbool.h>
#include "queue.h"
#include "rdm.h"

extern structQueueItem Queue[QUEUE_LENGTH];
extern uint8 QueueMemory[QUEUE_MEMORY];
extern uint8 QueueIndex;
extern uint8* QueueMemPtr;
extern uint8 QueueMemLen;

extern bool RDM_pendingDequeue;

bool queue_enqueue(uint16 parameterID, uint8 commandClass, uint8* Data, uint8 DataLength)
{
    structQueueItem QI;
    QI.uiParameterID = CYSWAP_ENDIAN16(parameterID);
    QI.byCommandClass = commandClass;

    if (DataLength > QueueMemLen || QueueIndex >= QUEUE_LENGTH)
        queue_dequeue();

    QI.byParameterData = QueueMemPtr;
    QI.byParameterLength = DataLength;
    memcpy(QueueMemPtr, Data, DataLength);

    QueueMemPtr += DataLength;
    QueueMemLen -= DataLength;

    Queue[QueueIndex] = QI;
    QueueIndex++;

    RDM_pendingDequeue = true;
    return true;
}

void queue_dequeue()
{
    uint8 memLen = Queue[0].byParameterLength;
    uint8 i;

    if (!QueueIndex)
        return;

    memmove(&Queue, &Queue[1], QUEUE_LENGTH - 1);
    memset(&Queue[QUEUE_LENGTH - 1], 0, sizeof(structQueueItem));

    memmove(QueueMemory, &QueueMemory[memLen], QUEUE_MEMORY - memLen);

    for (i = 0; i < QUEUE_LENGTH; i++)
        if (Queue[i].uiParameterID)
            Queue[i].byParameterData -= memLen;

    QueueMemPtr -= memLen;
    QueueMemLen += memLen;
    QueueIndex--;
}

structQueueItem queue_front()
{
    return Queue[0];
}

uint8 queue_count()
{
    return QueueIndex;
}
/* [] END OF FILE */
