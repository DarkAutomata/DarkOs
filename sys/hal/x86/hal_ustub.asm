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
; Kernel/HAL Pre-boot kernel initialization code.
; 

%include "hal_stub_inc.asm"

%define MULTIBOOT_MAGIC     0x1BADB002
%define MULTIBOOT_FLAGS     0x00010000
%define MULTIBOOT_XSUM      0x00-(MULTIBOOT_MAGIC+MULTIBOOT_FLAGS)

org HAL_PREBOOT_BASE

bits 32

Hal_PreBoot:
    jmp     Hal_PreBoot2

; The Multiboot header (required for GRUB to find us).
align 16, db 0x00
MultiBootTable:
    dd      MULTIBOOT_MAGIC ; Magic
    dd      MULTIBOOT_FLAGS ; Flags.
    dd      MULTIBOOT_XSUM  ; Checksum.
    dd      MultiBootTable  ; Header address.
    dd      HAL_LOAD_BASE   ; Load base.
    dd      0x00            ; Load end (0 = Full File)
    dd      0x00            ; BSS end (0 = No BSS)
    dd      HAL_LOAD_BASE   ; Entry address.
    dd      0x00            ; Video mode type.
    dd      0x00            ; Video width.
    dd      0x00            ; Video height.
    dd      0x00            ; Video depth.

; Machine info passed to the HalKernelEntry function.
align 16, db 0x00
xMachineInfo:
xMachineInfo_MultiBoot:     dd 0x00

; Include common CPU initialization code (Halx86_CpuInitialize).
%include "hal_cpuinit.asm"

Hal_PreBoot2:
    ; Save the Multiboot pointer.
    mov     [xMachineInfo_MultiBoot], ebx
    
    ; Initialize the CPU0 block, the GDT, and segments.
    mov     eax, HAL_CPU_BLOCK_BASE
    mov     ebp, Hal_PreBoot3       ; Return address... to avoid stack use.
    jmp     Halx86_CpuInitialize
    
Hal_PreBoot3:
    ; Now we know the segment environment we are working in.  Setup the
    ; stack pointer for thread 0 and then setup basic paging.
    mov     esp, HAL_BOOT_STACK_OFFSET
    
    ; Setup basic paging.
    ; Map:
    ;   00000000-003FFFFF = 00000000-003FFFFF   (first 4 MB).
    ;   00000000-00FFFFFF = 80000000-81FFFFFF   (first 32 MB)
    
    ; First, clear all the PDEs (set PRESENT=0 to clear).
    mov     eax, 0x00000000
    mov     edx, HAL_KERNEL_PDE_ADDR
    mov     ecx, 1024
    call    HalPage_FillPtes
    
    ; First PDE (first 4 MB).
    mov     eax, HAL_KERNEL_PTE_START + HAL_KERNEL_PDE_FLAGS
    mov     ecx, 1
    mov     edx, HAL_KERNEL_PDE_ADDR
    call    HalPage_FillPtes
    
    ; Now 8 PDEs for the first 32 MB of kernel space.
    mov     eax, HAL_KERNEL_PTE_START + HAL_KERNEL_PDE_FLAGS
    mov     ecx, 8
    mov     edx, HAL_KERNEL_PDE_ADDR + (512 * 4)    ; (512 * 4) --> 0x80000000
    call    HalPage_FillPtes
    
    ; Setup the page tables for lower 32 MB.
    ; We only fill out the first 4 MB PDE.  All 32 MB are mapped to the kernel
    ; address space.
    mov     edx, HAL_KERNEL_PTE_START + 0x0000
    mov     eax, 0x00000000 + HAL_KERNEL_PTE_FLAGS
    mov     ecx, 8*1024        ; 8K pages = 32MB.
    call    HalPage_FillPtes
    
    ; Enable paging.
    mov     eax, HAL_KERNEL_PDE_ADDR
    mov     cr3, eax
    mov     eax, cr0
    or      eax, 0x80000000
    mov     cr0, eax
    
    ; Set stack offset to higher VA.
    mov     eax, HAL_KERNEL_BASE_VA
    add     esp, eax
    
    push    xMachineInfo
    call    HAL_KERNEL_ENTRY_VA
    jmp     $

HalPage_FillPtes:
    ; eax has the page base address.
    ; ecx has the number of pages.
    ; edx has the page table start address.
    mov     [edx], eax
    add     edx, 4
    add     eax, 4096
    dec     ecx
    jne     HalPage_FillPtes
    ret

; Pad out this since we prepend it to the kernel.
times (HAL_PREBOOT_LEN)-($-$$) db 0x00


