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

; CPU initialization code.  This lives here so it can be used in both the
; PreBoot environment as well as the main kernel code.

; CPU initialization, DO NOT USE ANY STACK SPACE IN THIS ROUTINE.
; NOTE: This implies it cannot be directly called from C code.
Halx86_CpuInitialize:
    ; Parameters:
    ;   eax = CPU block base.
    ;   ebp = return address.
    
    ; Clear the entire block.
    mov     ecx, HAL_CPU_BLOCK_SIZE
    
Halx86_CpuInitialize_clear:
    mov     byte [eax], 0
    dec     ecx
    jnz     Halx86_CpuInitialize_clear
    
    ; Setup the self-pointer (so we can find it from the GS).
    mov     [eax], eax
    
    ; Setup the GDT pointer.
    mov     ecx, eax
    add     ecx, HALX86_CPU_BLOCK.Gdt_0
    mov     [eax+HALX86_CPU_BLOCK.Gdt_Addr], ecx
    
    ; Fill out the GDT.
    mov     word [eax+HALX86_CPU_BLOCK.Gdt_Len], (8*HAL_GDT_COUNT) - 1
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_0+0], 0
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_0+4], 0
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_1+0], HAL_KERNEL_CS_DW0
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_1+4], HAL_KERNEL_CS_DW1
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_2+0], HAL_KERNEL_DS_DW0
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_2+4], HAL_KERNEL_DS_DW1
    
    ; Setup the CPU base segment properly.
    mov     ebx, eax
    sal     ebx, 16             ; Lower 16 base bits are the upper 16 GDT bits
    and     ebx, 0xFFFF0000
    or      ebx, HAL_KERNEL_CPU_DW0
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_3+0], ebx
    
    mov     ebx, eax
    sar     ebx, 16             ; 16-23 base bits are lower 8 GDT bits.
    and     ebx, 0x00FF
    mov     ecx, eax            ; 24-32 base bits are upper 8 GDT bits.
    and     ecx, 0xFF000000
    or      ebx, ecx
    or      ebx, HAL_KERNEL_CPU_DW1
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_3+4], ebx
    
    ; Setup TSS properly from data.
    mov     ebx, eax
    add     ebx, HALX86_CPU_BLOCK.Tss_Start
    sal     ebx, 16             ; Lower 16 base bits are the upper 16 GDT bits
    and     ebx, 0xFFFF0000
    mov     ecx, HALX86_CPU_BLOCK.Tss_Length - HALX86_CPU_BLOCK.Tss_Start
    and     ecx, 0x0000FFFF     ; Limit (size) bits.
    or      ebx, ecx
    or      ebx, HAL_KERNEL_TSS_DW0
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_4+0], ebx
    
    mov     ebx, eax
    add     ebx, HALX86_CPU_BLOCK.Tss_Start
    
    sar     ebx, 16             ; 16-23 base bits are lower 8 GDT bits.
    and     ebx, 0x00FF
    
    mov     ecx, eax
    add     ecx, HALX86_CPU_BLOCK.Tss_Start
    mov     ecx, eax            ; 24-32 base bits are upper 8 GDT bits.
    and     ecx, 0xFF000000
    or      ebx, ecx
    or      ebx, HAL_KERNEL_TSS_DW1
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_4+4], ebx
    
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_5+0], HAL_USER_CS_DW0
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_5+4], HAL_USER_CS_DW1
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_6+0], HAL_USER_DS_DW0
    mov     dword [eax+HALX86_CPU_BLOCK.Gdt_6+4], HAL_USER_DS_DW1
    
    ; Load the GDT.
    lgdt    [eax+HALX86_CPU_BLOCK.Gdt_Start]
    
    jmp     HAL_KERNEL_CS:Halx86_CpuInitialize_FixSegs
    
Halx86_CpuInitialize_FixSegs:
    mov     ax, HAL_KERNEL_DS
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     ss, ax
    
    mov     ax, HAL_KERNEL_CPU
    mov     gs, ax
    
    mov     eax, ebp
    jmp     eax


