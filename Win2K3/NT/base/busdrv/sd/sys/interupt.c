/*++

Copyright (c) 2002 Microsoft Corporation

Module Name:

    interupt.c

Abstract:

    This module contains code to support events generated by the
    sd controller.

Author:

    Neil Sandlin (neilsa) 1-Jan-2002

Environment:

    Kernel mode

Revision History :

--*/

#include "pch.h"

VOID
SdbusEventWorkItemProc(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID Context
    );

VOID
SdbusReflectCardInterrupt(
    IN PDEVICE_OBJECT Fdo
    );



BOOLEAN
SdbusInterrupt(
   IN PKINTERRUPT InterruptObject,
   PVOID Context
   )

/*++

Routine Description:

    interrupt handler

Arguments:

    InterruptObject - Pointer to the interrupt object.
    Context - Pointer to the device context.

Return Value:

    Status

--*/

{
    PFDO_EXTENSION    fdoExtension;
    ULONG eventMask;
   
    fdoExtension=((PDEVICE_OBJECT)Context)->DeviceExtension;
   
    if (fdoExtension->Flags & SDBUS_FDO_OFFLINE) {
       return FALSE;
    }
   

    eventMask = (*(fdoExtension->FunctionBlock->GetPendingEvents))(fdoExtension);

    if (eventMask) {

        fdoExtension->IsrEventStatus |= eventMask;

        (*(fdoExtension->FunctionBlock->DisableEvent))(fdoExtension, eventMask);
        //
        // Something changed out there.. could be
        // a card insertion/removal.
        // Request a DPC to check it out.
        //
        IoRequestDpc((PDEVICE_OBJECT) Context, NULL, NULL);
    }
    return (eventMask != 0);
}


BOOLEAN
SdbusInterruptSynchronize(
    PFDO_EXTENSION fdoExtension
    )
{

    fdoExtension->LatchedIsrEventStatus = fdoExtension->IsrEventStatus;
    fdoExtension->IsrEventStatus = 0;
    return TRUE;
}    


VOID
SdbusInterruptDpc(
   IN PKDPC          Dpc,
   IN PDEVICE_OBJECT DeviceObject,
   IN PVOID          SystemContext1,
   IN PVOID          SystemContext2
   )

/*++

Routine Description:

    This DPC is just an intermediate step in getting to the main DPC
    handler. This is used to "debounce" hardware and give it some time after
    the physical interrupt has come in.

Arguments:

    DeviceObject - Pointer to the device object.

Return Value:


--*/

{
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    static ULONG IoWorkerEventTypes[] = {SDBUS_EVENT_CARD_RW_END,
                                         SDBUS_EVENT_BUFFER_EMPTY,
                                         SDBUS_EVENT_BUFFER_FULL,
                                         SDBUS_EVENT_CARD_RESPONSE,
                                         0};
    UCHAR i;
    ULONG acknowledgedEvents;
   
   
    if (fdoExtension->Flags & SDBUS_FDO_OFFLINE) {
       return;
    }
    

    KeSynchronizeExecution(fdoExtension->SdbusInterruptObject, SdbusInterruptSynchronize, fdoExtension);

    DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: Event Status %08x\n", fdoExtension->LatchedIsrEventStatus));

    if (fdoExtension->LatchedIsrEventStatus & SDBUS_EVENT_INSERTION) {

        DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: Card Insertion\n"));
   
        (*(fdoExtension->FunctionBlock->AcknowledgeEvent))(fdoExtension, SDBUS_EVENT_INSERTION);
        fdoExtension->LatchedIsrEventStatus &= ~SDBUS_EVENT_INSERTION;
        SdbusActivateSocket(DeviceObject, NULL, NULL);
    }        
        
    if (fdoExtension->LatchedIsrEventStatus & SDBUS_EVENT_REMOVAL) {

        DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: Card Removal\n"));
   
        (*(fdoExtension->FunctionBlock->AcknowledgeEvent))(fdoExtension, SDBUS_EVENT_REMOVAL);
        fdoExtension->LatchedIsrEventStatus &= ~SDBUS_EVENT_REMOVAL;
        SdbusActivateSocket(DeviceObject, NULL, NULL);
        
    }        

    acknowledgedEvents = 0;

    for(i = 0; IoWorkerEventTypes[i] != 0; i++) {
        if (fdoExtension->LatchedIsrEventStatus & IoWorkerEventTypes[i]) {

            DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: received event %08x - %s\n",
                        IoWorkerEventTypes[i], EVENT_STRING(IoWorkerEventTypes[i])));
        
            (*(fdoExtension->FunctionBlock->AcknowledgeEvent))(fdoExtension, IoWorkerEventTypes[i]);

            DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: ack'd event %08x - %s\n",
                        IoWorkerEventTypes[i], EVENT_STRING(IoWorkerEventTypes[i])));

            acknowledgedEvents |= IoWorkerEventTypes[i];
            fdoExtension->LatchedIsrEventStatus &= ~IoWorkerEventTypes[i];
        }        
    }
    
    
    if (acknowledgedEvents) {
    
        DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: dispatching event %08x\n", acknowledgedEvents));
        SdbusPushWorkerEvent(fdoExtension, acknowledgedEvents);
    }
    
    //
    // Now check to see if the card has interrupted, and call back to the function driver
    //
    if (fdoExtension->LatchedIsrEventStatus & SDBUS_EVENT_CARD_INTERRUPT) {
    
        DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: got CARD INTERRUPT\n"));

        SdbusReflectCardInterrupt(DeviceObject);

        fdoExtension->LatchedIsrEventStatus &= ~SDBUS_EVENT_CARD_INTERRUPT;
    }
    
    ASSERT(fdoExtension->LatchedIsrEventStatus == 0);

    return;
}   




VOID
SdbusReflectCardInterrupt(
    IN PDEVICE_OBJECT Fdo
    )
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PDEVICE_OBJECT pdo;
    PPDO_EXTENSION pdoExtension;
    
    //
    // ISSUE: NEED TO IMPLEMENT:
    //  need to queue an IoWorker packet to read the CCCR and find out which
    //  function is interrupting. For now, assume there is only a single I/O function,
    //  and use the first callback found.
    //        
    for (pdo = fdoExtension->PdoList; pdo != NULL; pdo = pdoExtension->NextPdoInFdoChain) {
        pdoExtension = pdo->DeviceExtension;
        
        if (pdoExtension->Flags & SDBUS_PDO_GENERATES_IRQ) {
        
        
            if (pdoExtension->Flags & SDBUS_PDO_DPC_CALLBACK) {
            
                pdoExtension->Flags |= SDBUS_PDO_CALLBACK_IN_SERVICE;
                (pdoExtension->CallbackRoutine)(pdoExtension->CallbackRoutineContext, 0);
                
            } else {
                pdoExtension->Flags |= SDBUS_PDO_CALLBACK_REQUESTED;
                KeSetEvent(&fdoExtension->CardInterruptEvent, 0, FALSE);
            }                
        }
    }
}    

VOID
SdbusEventWorkItemProc(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID Context
    )
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PKEVENT Events[2] = {&fdoExtension->CardInterruptEvent, &fdoExtension->WorkItemExitEvent};
    PDEVICE_OBJECT pdo;
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;

    while(TRUE){
    
        status = KeWaitForMultipleObjects(2, Events, WaitAny,
                                          Executive, KernelMode, FALSE,
                                          NULL, NULL);

        if ((fdoExtension->Flags & SDBUS_FDO_WORK_ITEM_ACTIVE) == 0) {
            break;
        }
        
        for (pdo = fdoExtension->PdoList; pdo != NULL; pdo = pdoExtension->NextPdoInFdoChain) {
            pdoExtension = pdo->DeviceExtension;
            
            if (pdoExtension->Flags & SDBUS_PDO_GENERATES_IRQ) {
            
                DebugPrint((SDBUS_DEBUG_CARD_EVT, "WorkItemProc: CallBack %08x %08x\n", pdoExtension->CallbackRoutine,
                                                    pdoExtension->CallbackRoutineContext));
                
                ASSERT((pdoExtension->Flags & SDBUS_PDO_CALLBACK_IN_SERVICE) == 0);
                
                pdoExtension->Flags |= SDBUS_PDO_CALLBACK_IN_SERVICE;
                pdoExtension->Flags &= ~SDBUS_PDO_CALLBACK_REQUESTED;
                (pdoExtension->CallbackRoutine)(pdoExtension->CallbackRoutineContext, 0);
                
            }
            
        }
    }

}