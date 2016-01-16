/*
Copyright (c) 2016, Jonathan Ward
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <krn_base.h>
#include <hal_common.h>
#include "hal_x86.h"

HALX86_MACHINE_INFO* g_pHalMachInfo = NULL;

// This is in defined in x86/hal_common.c.
void
OSCALL
Hal_ConsoleInit(
    char* pBuffer,
    uint32_t Width,
    uint32_t Height
    );

void
OSCALL
Hal_TestDataStructures(
    );

void
OSCALL
Hal_DumpGdtEntry(
    uint32_t Offset,
    HALX86_GDT_ENTRY* pEntry
    )
{
    uint32_t limit;
    
    limit = HALX86_GET_GDT_LIMIT(*pEntry);
    if (pEntry->Flags.Granularity)
    {
        limit = (limit << 12) | 0x0FFF;
    }
    
    // TODO: This doesn't handle system segments properly.
    Hal_conprintf(
            "GDT: %02X:%2d-bit:%s:%08X+%08X:R[%01d]:%s:%s%s%s\n",
            Offset,
            (pEntry->Flags.Size ? 32 : 16),
            (pEntry->Access.Code ? "CODE" : "DATA"),
            HALX86_GET_GDT_BASE(*pEntry),
            limit,
            pEntry->Access.RingLevel,
            (pEntry->Flags.Granularity ? "4K" : "1B"),
            (pEntry->Access.ReadWrite ? "W" : " "),
            (pEntry->Access.DC ? "D" : " "),
            (pEntry->Access.Present ? "V" : " "));
}

void
OSCALL
Hal_DumpIdtEntry(
    HALX86_IDT_ENTRY* pEntry
    )
{
}

void
OSCALL
Hal_KernelEntry(
    HALX86_MACHINE_INFO* pContext
    )
{
    g_pHalMachInfo = pContext;
    
    Hal_ConsoleInit((char*) 0x000B8000, 80, 25);
    Hal_conprintf("DarkOS 0.0.1\n");
    Hal_conprintf("Machine Info: 0x%p\n", pContext);
    
    {
        HALX86_CPU_BLOCK* pCpu;
        uint32_t i;
        
        pCpu = Halx86_GetCurrentCpuBlock();
        
        Hal_conprintf("GDT Count: %u\n", (pCpu->Gdt_Len + 1) / 8);
        
        for (i = 1; i < (pCpu->Gdt_Len + 1) / 8; i++)
        {
            Hal_DumpGdtEntry(i * 8, &pCpu->pGdt_Addr[i]);
        }
    }
    
    // Let's test the data structure functionality.
    // TODO: Eventually, we can remove this, but it's a good unit test.
    Hal_TestDataStructures();
    
    Hal_conprintf("Initializing IDT\n");
    Halx86_InitIdt();
    
    Hal_conprintf("Configuring PIC\n");
    Halx86_PicInit();
    
    Hal_conprintf("Enabling interrupts\n");
    Halx86_sti();
    
    // Enable the timer interrupt.
    Halx86_PicEnableDevices(0x01);
    
    // Do something.
    while(1);
}

void
OSCALL
Halx86_PicInit(
    )
{
    Halx86_outb(
            HALX86_PIC1_CMD,
            HALX86_PIC_ICW1_INIT + HALX86_PIC_ICW1_ICW4);
    Halx86_BusySleep(1024);
    
    Halx86_outb(
            HALX86_PIC2_CMD,
            HALX86_PIC_ICW1_INIT + HALX86_PIC_ICW1_ICW4);
    Halx86_BusySleep(1024);
    
    Halx86_outb(
            HALX86_PIC1_DATA,
            0x20);
    Halx86_BusySleep(1024);
    
    Halx86_outb(
            HALX86_PIC2_DATA,
            0x28);
    Halx86_BusySleep(1024);
    
    Halx86_outb(
            HALX86_PIC1_DATA,
            4);
    Halx86_BusySleep(1024);
    
    Halx86_outb(
            HALX86_PIC2_DATA,
            2);
    Halx86_BusySleep(1024);
    
    Halx86_outb(
            HALX86_PIC1_DATA,
            HALX86_PIC_ICW4_8086);
    Halx86_BusySleep(1024);
    
    Halx86_outb(
            HALX86_PIC2_DATA,
            HALX86_PIC_ICW4_8086);
    Halx86_BusySleep(1024);
    
    // Initialize masks to nothing.
    Halx86_outb(
            HALX86_PIC1_DATA,
            0xFF);
    Halx86_outb(
            HALX86_PIC2_DATA,
            0xFF);
}

uint32_t g_Halx86_busyCounter = 0;

uint32_t
OSCALL
Halx86_BusySleep(
    uint32_t Count
    )
{
    while (Count-- > 0)
    {
        g_Halx86_busyCounter++;
    }
    
    return g_Halx86_busyCounter;
}

void
OSCALL
Halx86_PicDisableDevices(
    uint32_t Mask
    )
{
    uint32_t values;
    
    values = 
        (uint32_t) Halx86_inb(HALX86_PIC1_DATA) |
        (((uint32_t) Halx86_inb(HALX86_PIC2_DATA)) << 8);
    
    // The way the PIC works, a set mask bit disables the IRQ.
    values |= Mask;
    
    Halx86_outb(HALX86_PIC1_DATA, (uint8_t) (values & 0x00FF));
    Halx86_outb(HALX86_PIC2_DATA, (uint8_t) ((values >> 8) & 0x00FF));
}

void
OSCALL
Halx86_PicEnableDevices(
    uint32_t Mask
    )
{
    uint32_t values;
    
    values = 
        (uint32_t) Halx86_inb(HALX86_PIC1_DATA) |
        ((uint32_t) Halx86_inb(HALX86_PIC2_DATA) << 8);
    
    // The way the PIC works, a clear mask bit enables the IRQ.
    values &= ~Mask;
    
    Halx86_outb(HALX86_PIC1_DATA, (uint8_t) (values & 0x00FF));
    Halx86_outb(HALX86_PIC2_DATA, (uint8_t) ((values >> 8) & 0x00FF));
}

void
OSCALL
Halx86_PicSendEoi(
    uint32_t IrqIndex
    )
{
    if (IrqIndex >= 8)
    {
        Halx86_outb(HALX86_PIC2_CMD, HALX86_PIC_EOI);
    }
    
    Halx86_outb(HALX86_PIC1_CMD, HALX86_PIC_EOI);
}

uint32_t
OSCALL
Halx86_GetPicIrqMask(
    )
{
    uint32_t values;
    
    Halx86_outb(HALX86_PIC1_CMD, HALX86_PIC_READ_ISR);
    Halx86_outb(HALX86_PIC2_CMD, HALX86_PIC_READ_ISR);
    
    values = 
        (uint32_t) Halx86_inb(HALX86_PIC1_CMD) |
        (((uint32_t) Halx86_inb(HALX86_PIC2_CMD)) << 8);
    
    return values;
}

uint32_t g_TimerHitCount = 0;

void
OSCALL
Halx86_IsrRootCallback(
    uint32_t IntIndex,
    uint32_t IntErrorCode,
    HALX86_CONTEXT_RECORD* pContext
    )
{
    // Hal_conprintf("Halx86_IsrRootCallback: Id=%02X, EC=%08X\n", IntIndex, IntErrorCode);
    
    // TODO: Handle the interrupt.
    if (IntIndex == 0x20)
    {
        // Timer.
        g_TimerHitCount++;
        if (0 == (g_TimerHitCount % 100))
        {
            Hal_conprintf("Halx86_TIMER: %d\n", g_TimerHitCount);
        }
    }
    
    // Send EOI when appropriate.
    // If this was an IRQ check the IRQ mask.
    if ((IntIndex >= 0x20) &&
        (IntIndex <= 0x2F))
    {
        uint32_t activeMask;
        
        IntIndex -= 0x20;
        
        activeMask = Halx86_GetPicIrqMask();
        
        if (activeMask & (1UL << IntIndex))
        {
            // Send EOI.
            Halx86_PicSendEoi(IntIndex);
        }
        else
        {
            if (IntIndex >= 8)
            {
                // Send the master an EOI for the spurious secondary.
                Halx86_PicSendEoi(2);
            }
        }
    }
}


void
OSCALL
Hal_TestDataStructures(
    )
{
    // Test out the stack functions.
    {
        KRN_STACK_ENTRY values[32];
        KRN_STACK_ENTRY head;
        
        Krn_StackInit(&head);
        
        Krn_StackPush(&head, &values[0]);
        Krn_StackPush(&head, &values[1]);
        Krn_StackPush(&head, &values[2]);
        
        if (Krn_StackPop(&head) != &values[2]) { Hal_conprintf("Stack Fail %d\n", __LINE__); while(1); }
        if (Krn_StackPop(&head) != &values[1]) { Hal_conprintf("Stack Fail %d\n", __LINE__); while(1); }
        if (Krn_StackPop(&head) != &values[0]) { Hal_conprintf("Stack Fail %d\n", __LINE__); while(1); }
        if (Krn_StackPop(&head) != NULL) { Hal_conprintf("Stack Fail %d\n"); while(1); }
        
        Krn_StackPush(&head, &values[3]);
        Krn_StackPush(&head, &values[4]);
        Krn_StackPush(&head, &values[5]);
        Krn_StackPush(&head, &values[6]);
        
        if (Krn_StackPop(&head) != &values[6]) { Hal_conprintf("Stack Fail %d\n", __LINE__); while(1); }
        if (Krn_StackPop(&head) != &values[5]) { Hal_conprintf("Stack Fail %d\n", __LINE__); while(1); }
        if (Krn_StackPop(&head) != &values[4]) { Hal_conprintf("Stack Fail %d\n", __LINE__); while(1); }
        if (Krn_StackPop(&head) != &values[3]) { Hal_conprintf("Stack Fail %d\n", __LINE__); while(1); }
        if (Krn_StackPop(&head) != NULL) { Hal_conprintf("Stack Fail 4\n"); while(1); }
        
        // Hal_conprintf("Stack passed\n");
    }
    
    // Test out the list functions.
    {
        KRN_LIST_ENTRY values[32];
        KRN_LIST_ENTRY head;
        
        Krn_ListInit(&head);
        
        if (Krn_ListRemoveHead(&head) != NULL) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveTail(&head) != NULL) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        
        Krn_ListAddHead(&head, &values[0]);
        if (Krn_ListRemoveHead(&head) != &values[0]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        
        Krn_ListAddHead(&head, &values[0]);
        if (Krn_ListRemoveTail(&head) != &values[0]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        
        Krn_ListAddTail(&head, &values[0]);
        if (Krn_ListRemoveHead(&head) != &values[0]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        
        Krn_ListAddTail(&head, &values[0]);
        if (Krn_ListRemoveTail(&head) != &values[0]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        
        if (Krn_ListRemoveHead(&head) != NULL) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveTail(&head) != NULL) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        
        Krn_ListAddTail(&head, &values[0]);
        Krn_ListAddTail(&head, &values[1]);
        Krn_ListAddTail(&head, &values[2]);
        Krn_ListAddTail(&head, &values[3]);
        
        if (Krn_ListRemoveHead(&head) != &values[0]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveHead(&head) != &values[1]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveHead(&head) != &values[2]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveHead(&head) != &values[3]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        
        Krn_ListAddHead(&head, &values[0]);
        Krn_ListAddHead(&head, &values[1]);
        Krn_ListAddHead(&head, &values[2]);
        Krn_ListAddHead(&head, &values[3]);
        
        if (Krn_ListRemoveTail(&head) != &values[0]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveTail(&head) != &values[1]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveTail(&head) != &values[2]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveTail(&head) != &values[3]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        
        Krn_ListAddTail(&head, &values[2]);
        Krn_ListAddTail(&head, &values[3]);
        Krn_ListAddHead(&head, &values[1]);
        Krn_ListAddHead(&head, &values[0]);
        
        if (Krn_ListRemoveHead(&head) != &values[0]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveHead(&head) != &values[1]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveHead(&head) != &values[2]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        if (Krn_ListRemoveHead(&head) != &values[3]) { Hal_conprintf("List Fail %d\n", __LINE__); while(1); }
        
        // Hal_conprintf("List Passed\n");
    }
}

