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

#ifndef __HAL_X86_H__
#define __HAL_X86_H__

#include <krn_base.h>


// Forward declarations.
typedef struct _HALX86_CPU_BLOCK HALX86_CPU_BLOCK;
typedef struct _HALX86_MACHINE_INFO HALX86_MACHINE_INFO;
typedef struct _HALX86_BIOS_INTERRUPT_CONTEXT HALX86_BIOS_INTERRUPT_CONTEXT;
typedef struct _HALX86_MULTIBOOT_INFO HALX86_MULTIBOOT_INFO;
typedef struct _HALX86_GDT_DESCRIPTOR HALX86_GDT_DESCRIPTOR;
typedef struct _HALX86_GDT_ENTRY HALX86_GDT_ENTRY;
typedef struct _HALX86_IDT_DESCRIPTOR HALX86_IDT_DESCRIPTOR;
typedef struct _HALX86_IDT_ENTRY HALX86_IDT_ENTRY;
typedef struct _HALX86_CONTEXT_RECORD HALX86_CONTEXT_RECORD;

typedef void (*HALX86_BIOS_INTERRUPT_CALLBACK)();


#pragma pack(push, 1)

struct _HALX86_CPU_BLOCK
{
    void* pCpuBlockPointer;
    uint32_t CpuBlockPointer_Pad[3];
    
    // The GDT and GDT table.
    uint16_t Gdt_Len;
    HALX86_GDT_ENTRY* pGdt_Addr;
    uint16_t Gdt_Pad;
    
    uint8_t Gdt_0[8];
    uint8_t Gdt_1[8];
    uint8_t Gdt_2[8];
    uint8_t Gdt_3[8];
    uint8_t Gdt_4[8];
    uint8_t Gdt_5[8];
    uint8_t Gdt_6[8];
    uint8_t Gdt_X[40];
    
    // TSS, align on 4-byte boundary.
    uint16_t Tss_Link;
    uint16_t Tss_Link_Pad;
    uint32_t Tss_Esp0;
    uint16_t Tss_Ss0;
    uint16_t Tss_Ss0_Pad;
    uint32_t Tss_Esp1;
    uint16_t Tss_Ss1;
    uint16_t Tss_Ss1_Pad;
    uint32_t Tss_Esp2;
    uint16_t Tss_Ss2;
    uint16_t Tss_Ss2_Pad;
    uint32_t Tss_Cr3;
    uint32_t Tss_Eip;
    uint32_t Tss_Eflags;
    uint32_t Tss_Eax;
    uint32_t Tss_Ecx;
    uint32_t Tss_Edx;
    uint32_t Tss_Ebx;
    uint32_t Tss_Esp;
    uint32_t Tss_Ebp;
    uint32_t Tss_Esi;
    uint32_t Tss_Edi;
    uint16_t Tss_Es;
    uint16_t Tss_Es_Pad;
    uint16_t Tss_Cs;
    uint16_t Tss_Cs_Pad;
    uint16_t Tss_Ss;
    uint16_t Tss_Ss_Pad;
    uint16_t Tss_Ds;
    uint16_t Tss_Ds_Pad;
    uint16_t Tss_Fs;
    uint16_t Tss_Fs_Pad;
    uint16_t Tss_Gs;
    uint16_t Tss_Gs_Pad;
    uint16_t Tss_Ldt;
    uint16_t Tss_Ldt_Pad;
    uint16_t Tss_IoBmpBase_Pad;
    uint16_t Tss_IoBmpBase;
    
    // Current execution state.
    void* pCurrentProcessBlock;
    void* pCurrentThreadBlock;
};

struct _HALX86_MACHINE_INFO
{
    HALX86_MULTIBOOT_INFO* pMultiBoot;
    void* pTmpData;
    uint32_t TmpDataLength;
    HALX86_BIOS_INTERRUPT_CONTEXT* pIntData;
    HALX86_BIOS_INTERRUPT_CALLBACK pIntFunc;
};

struct _HALX86_BIOS_INTERRUPT_CONTEXT
{
    uint16_t IntID;
    uint16_t AX;
    uint16_t BX;
    uint16_t CX;
    uint16_t DX;
    uint16_t SI;
    uint16_t DI;
    uint16_t BP;
    uint16_t ES;
    uint16_t CF;
};

struct _HALX86_MULTIBOOT_INFO
{
    uint32_t Flags;
    uint32_t MemLower;
    uint32_t MemUpper;
    uint32_t BootDevice;
    uint32_t CmdLine;
    uint32_t ModsCount;
    uint32_t ModsAddress;
    uint32_t Syms[4];
    uint32_t MmapLength;
    uint32_t MmapAddress;
    uint32_t DrivesLength;
    uint32_t DrivesAddress;
    uint32_t ConfigTable;
    uint32_t BootLoaderName;
    uint32_t ApmTable;
    uint32_t VbeControlInfo;
    uint32_t VbeModeInfo;
    uint32_t VbeMode;
    uint32_t VbeInterfaceSegment;
    uint32_t VbeInterfaceOffset;
    uint32_t VbeInterfaceLength;
};

struct _HALX86_GDT_DESCRIPTOR
{
    uint16_t SizeMinus1;
    HALX86_GDT_ENTRY* pTable;
};

typedef struct _HALX86_GDT_ACCESS_BYTE
{
    uint8_t Accessed    : 1;    // Written by CPU
    uint8_t ReadWrite   : 1;    // Code-read or data-write
    uint8_t DC          : 1;    // Direction/Conforming bit (see docs)
    uint8_t Code        : 1;    // This is a code segment
    uint8_t IsSegment   : 1;    // 0 = system descriptor, 1 = segment
    uint8_t RingLevel   : 2;    // 0-3
    uint8_t Present     : 1;    // Must be 1
} HALX86_GDT_ACCESS_BYTE;

typedef struct _HALX86_GDT_FLAGS_BYTE
{
    uint8_t Limit_16_19 : 4;    // Upper nibble of limit
    uint8_t Reserved0   : 2;    // Must be 0
    uint8_t Granularity : 1;    // 0 = 1 byte selector, 1 4K selector
    uint8_t Size        : 1;    // 0 = 16 bit, 1 = 32 bit
} HALX86_GDT_FLAGS_BYTE;

struct _HALX86_GDT_ENTRY
{
    uint16_t Limit_00_15;
    uint16_t Base_00_15;
    uint8_t Base_16_23;
    HALX86_GDT_ACCESS_BYTE Access;
    HALX86_GDT_FLAGS_BYTE Flags;
    uint8_t Base_24_31;
};

#define HALX86_GET_GDT_BASE(GDT) ( \
    ((uint32_t)(((GDT).Base_00_15) & 0xFFFF) << 0)  | \
    ((uint32_t)(((GDT).Base_16_23) & 0x00FF) << 16) | \
    ((uint32_t)(((GDT).Base_24_31) & 0x00FF) << 24) )

#define HALX86_GET_GDT_LIMIT(GDT) ( \
    ((uint32_t)(((GDT).Limit_00_15) & 0xFFFF) << 0)      | \
    ((uint32_t)(((GDT).Flags.Limit_16_19) & 0x0F) << 16) )

#define HALX86_SET_GDT_BASE(GDT, BASE) \
    (GDT).Base_00_15 = (uint16_t)(((BASE) >> 0) & 0xFFFF); \
    (GDT).Base_16_23 = (uint8_t )(((BASE) >> 16) & 0xFF); \
    (GDT).Base_24_31 = (uint8_t )(((BASE) >> 24) & 0xFF);

#define HALX86_SET_GDT_LIMIT(GDT, LIMIT) \
    (GDT).Limit_00_15       = (uint16_t)(((LIMIT) >> 0) & 0xFFFF); \
    (GDT).Flags.Limit_16_19 = (uint8_t )(((LIMIT) >> 16) & 0xFF);

struct _HALX86_IDT_DESCRIPTOR
{
    uint16_t SizeMinus1;
    HALX86_IDT_ENTRY* pTable;
};

struct _HALX86_IDT_ENTRY
{
    uint16_t OffsetLow;
    uint16_t Selector;
    uint8_t Reserved0;
    uint8_t Type;
    uint16_t OffsetHigh;
};

#define HALX86_PDE_FLAG_PRESENT     0x00000001
#define HALX86_PDE_FLAG_RW          0x00000002
#define HALX86_PDE_FLAG_USER        0x00000004
#define HALX86_PDE_FLAG_PWT         0x00000008
#define HALX86_PDE_FLAG_PCD         0x00000010
#define HALX86_PDE_FLAG_ACCESSED    0x00000020
#define HALX86_PDE_FLAG_IGNORED0    0x00000040
#define HALX86_PDE_FLAG_4MB         0x00000080
#define HALX86_PDE_FLAG_IGNORED1    0x00000100
#define HALX86_PDE_MASK_SOFTWARE    0x00000E00
#define HALX86_PDE_MASK_PTE_BASE    0xFFFFF000

#define HALX86_PTE_FLAG_PRESENT     0x00000001
#define HALX86_PTE_FLAG_RW          0x00000002
#define HALX86_PTE_FLAG_USER        0x00000004
#define HALX86_PTE_FLAG_PWT         0x00000008
#define HALX86_PTE_FLAG_PCD         0x00000010
#define HALX86_PTE_FLAG_ACCESSED    0x00000020
#define HALX86_PTE_FLAG_DIRTY       0x00000040
#define HALX86_PTE_FLAG_PAT         0x00000080
#define HALX86_PTE_FLAG_GLOBAL      0x00000100
#define HALX86_PTE_MASK_SOFTWARE    0x00000E00
#define HALX86_PTE_MASK_BASE_ADDR   0xFFFFF000

struct _HALX86_CONTEXT_RECORD
{
    // Execution context.
    uint32_t RegEip;
    uint32_t RegEflags;
    uint32_t RegEbp;
    uint32_t RegEsp;
    uint32_t RegCr3;
    
    // General registers.
    uint32_t RegEax;
    uint32_t RegEbx;
    uint32_t RegEcx;
    uint32_t RegEdx;
    uint32_t RegEdi;
    uint32_t RegEsi;
    
    // Segment registers.
    uint16_t RegCs;
    uint16_t RegDs;
    uint16_t RegEs;
    uint16_t RegFs;
    uint16_t RegGs;
    uint16_t RegSs;
};

#pragma pack(pop)   // pack(1)

#define HALX86_PIC1_CMD             0x20
#define HALX86_PIC1_DATA            0x21
#define HALX86_PIC2_CMD             0xA0
#define HALX86_PIC2_DATA            0xA1
#define HALX86_PIC_READ_IRR         0x0A
#define HALX86_PIC_READ_ISR         0x0B
#define HALX86_PIC_EOI              0x20

#define HALX86_PIC_ICW1_ICW4        0x01
#define HALX86_PIC_ICW1_SINGLE      0x02
#define HALX86_PIC_ICW1_INTERVAL4   0x04
#define HALX86_PIC_ICW1_LEVEL       0x08
#define HALX86_PIC_ICW1_INIT        0x10

#define HALX86_PIC_ICW4_8086        0x01
#define HALX86_PIC_ICW4_AUTO        0x02
#define HALX86_PIC_ICW4_BUFF_SLAVE  0x08
#define HALX86_PIC_ICW4_BUFF_MASTER 0x0C
#define HALX86_PIC_ICW4_SFNM        0x10


extern HALX86_MACHINE_INFO* g_pHalx86_MachInfo;

HALX86_CPU_BLOCK*
OSCALL
Halx86_GetCurrentCpuBlock(
    );

uint8_t
OSCALL
Halx86_inb(
    uint16_t PortAddress
    );

uint16_t
OSCALL
Halx86_inw(
    uint16_t PortAddress
    );

uint32_t
OSCALL
Halx86_indw(
    uint16_t PortAddress
    );

void
OSCALL
Halx86_outb(
    uint16_t PortAddress,
    uint8_t Value
    );

void
OSCALL
Halx86_outw(
    uint16_t PortAddress,
    uint16_t Value
    );

void
OSCALL
Halx86_outdw(
    uint16_t PortAddress,
    uint32_t Value
    );

void
OSCALL
Halx86_sti(
    );

void
OSCALL
Halx86_cli(
    );

void
OSCALL
Halx86_InitIdt(
    );

void
OSCALL
Halx86_PicInit(
    );

uint32_t
OSCALL
Halx86_BusySleep(
    uint32_t Count
    );

void
OSCALL
Halx86_PicDisableDevices(
    uint32_t Mask
    );

void
OSCALL
Halx86_PicEnableDevices(
    uint32_t Mask
    );

void
OSCALL
Halx86_PicSendEoi(
    uint32_t IrqIndex
    );

uint32_t
OSCALL
Halx86_PicGetIrqMask(
    );

void
OSCALL
Halx86_IsrRootCallback(
    uint32_t IntIndex,
    uint32_t IntErrorCode,
    HALX86_CONTEXT_RECORD* pContext
    );

#endif // __HAL_X86_H__

