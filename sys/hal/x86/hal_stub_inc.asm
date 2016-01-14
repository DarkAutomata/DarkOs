; Copyright (c) 2016, Jonathan Ward
; All rights reserved.
; 
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
; 
; * Redistributions of source code must retain the above copyright notice, this
;   list of conditions and the following disclaimer.
; 
; * Redistributions in binary form must reproduce the above copyright notice,
;   this list of conditions and the following disclaimer in the documentation
;   and/or other materials provided with the distribution.
; 
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

; 
; HAL common upper stub definitions.
; 

; HAL / Kernel binary layout:
;   HAL_UPPER   - Provides entry point at load base, support routines.
;   Kernel      - Linked "Kernel" (actually, still HAL) entry.

%define MULTIBOOT_V1_MAGIC      0x1BADB002
%define MULTIBOOT_V1_FLAGS      0x00010000
%define MULTIBOOT_V1_XSUM       0x00-(MULTIBOOT_V1_MAGIC+MULTIBOOT_V1_FLAGS)

%define HAL_LOAD_BASE           0x00100000
%define HAL_PREBOOT_BASE        0x00100000
%define HAL_PREBOOT_LEN         0x00008000
%define HAL_KERNEL_BASE_VA      0x80000000
%define HAL_KERNEL_ENTRY_IMG    (HAL_PREBOOT_BASE + HAL_PREBOOT_LEN)
%define HAL_KERNEL_ENTRY_VA     (HAL_KERNEL_BASE_VA + HAL_KERNEL_ENTRY_IMG)

%define HAL_BOOT_STACK_OFFSET   0x0008FFF0

%define HAL_IDT_TABLE           0x9000
%define HAL_IDT_TABLE_COUNT     256
%define HAL_IDT_TABLE_LEN       (8 * HAL_IDT_TABLE_COUNT)
%define HAL_IDT                 0x9900
%define HAL_IDT_16              0x9910
%define HAL_BIOS_INTEROP        0x9A00

%define HAL_KERNEL_PDE_ADDR     0x00020000
%define HAL_KERNEL_PTE_START    0x00021000
%define HAL_KERNEL_PTE_LEN      0x00008000  ; Identity maps firstst 32 MB of RAM to kernel.
%define HAL_KERNEL_PDE_FLAGS    0x00000003
%define HAL_KERNEL_PTE_FLAGS    0x00000003

%define HAL_KERNEL_CS_DW0       0x0000FFFF
%define HAL_KERNEL_CS_DW1       0x00CF9A00

%define HAL_KERNEL_DS_DW0       0x0000FFFF
%define HAL_KERNEL_DS_DW1       0x00CF9200

%define HAL_KERNEL_CPU_DW0      0x0000FFFF
%define HAL_KERNEL_CPU_DW1      0x00C09200

%define HAL_KERNEL_TSS_DW0      0x00000000
%define HAL_KERNEL_TSS_DW1      0x00008900

%define HAL_USER_CS_DW0         0x0000FFFF
%define HAL_USER_CS_DW1         0x00CFFA00

%define HAL_USER_DS_DW0         0x0000FFFF
%define HAL_USER_DS_DW1         0x00CFF200

%define HAL_KERNEL_CS           (8 * 1)
%define HAL_KERNEL_DS           (8 * 2)
%define HAL_KERNEL_CPU          (8 * 3)
%define HAL_KERNEL_TSS          (8 * 4)
%define HAL_USER_CS             (8 * 5)
%define HAL_USER_DS             (8 * 6)
%define HAL_GDT_COUNT           7

%define HAL_CPU_BLOCK_BASE      0x00030000
%define HAL_CPU_BLOCK_SIZE      0x00001000

struc HALX86_CPU_BLOCK
    .CpuBlockPointer            resd 1
    .CpuBlockPointer_Pad        resd 3  ; Align on 16-byte boundary.
    
    ; The GDT and GDT table.
    .Gdt_Start:
    .Gdt_Len                    resw 1
    .Gdt_Addr                   resd 1
    .Gdt_Pad                    resw 1
    
    .Gdt_0                      resb 8
    .Gdt_1                      resb 8
    .Gdt_2                      resb 8
    .Gdt_3                      resb 8
    .Gdt_4                      resb 8
    .Gdt_5                      resb 8
    .Gdt_6                      resb 8
    .Gdt_X                      resb 40
    
    ; TSS, align on 4-byte boundary.
    .Tss_Start:
    .Tss_Link                   resw 1  ; 0x00
    .Tss_Link_Pad               resw 1  ; 0x02
    .Tss_Esp0                   resd 1  ; 0x04
    .Tss_Ss0                    resw 1  ; 0x08
    .Tss_Ss0_Pad                resw 1  ; 0x0A
    .Tss_Esp1                   resd 1  ; 0x0C
    .Tss_Ss1                    resw 1  ; 0x10
    .Tss_Ss1_Pad                resw 1  ; 0x12
    .Tss_Esp2                   resd 1  ; 0x14
    .Tss_Ss2                    resw 1  ; 0x18
    .Tss_Ss2_Pad                resw 1  ; 0x1A
    .Tss_Cr3                    resd 1  ; 0x1C
    .Tss_Eip                    resd 1  ; 0x20
    .Tss_Eflags                 resd 1  ; 0x24
    .Tss_Eax                    resd 1  ; 0x28
    .Tss_Ecx                    resd 1  ; 0x2C
    .Tss_Edx                    resd 1  ; 0x30
    .Tss_Ebx                    resd 1  ; 0x34
    .Tss_Esp                    resd 1  ; 0x38
    .Tss_Ebp                    resd 1  ; 0x3C
    .Tss_Esi                    resd 1  ; 0x40
    .Tss_Edi                    resd 1  ; 0x44
    .Tss_Es                     resw 1  ; 0x48
    .Tss_Es_Pad                 resw 1  ; 0x4A
    .Tss_Cs                     resw 1  ; 0x4C
    .Tss_Cs_Pad                 resw 1  ; 0x4E
    .Tss_Ss                     resw 1  ; 0x50
    .Tss_Ss_Pad                 resw 1  ; 0x52
    .Tss_Ds                     resw 1  ; 0x54
    .Tss_Ds_Pad                 resw 1  ; 0x56
    .Tss_Fs                     resw 1  ; 0x58
    .Tss_Fs_Pad                 resw 1  ; 0x5A
    .Tss_Gs                     resw 1  ; 0x5C
    .Tss_Gs_Pad                 resw 1  ; 0x5E
    .Tss_Ldt                    resw 1  ; 0x60
    .Tss_Ldt_Pad                resw 1  ; 0x62
    .Tss_IoBmpBase_Pad          resw 1  ; 0x64
    .Tss_IoBmpBase              resw 1  ; 0x66
    .Tss_Length:
    
    ; Current execution state.
    .CurrentProcessBlock        resd 1
    .CurrentThreadBlock         resd 1
    
    ; Size
    .size:
endstruc

; Interrupt support structures.
struc HALX86_CONTEXT_RECORD
    .RegEsp     resd 1      ; 0x00
    .RegCr3     resd 1      ; 0x04
    .size:                  ; 0x08
endstruc


