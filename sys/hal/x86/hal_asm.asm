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
; HAL assembly support routines.
; 

%include "hal_stub_inc.asm"

bits 32

; Imports.
extern Hal_KernelEntry
extern Halx86_IsrRootCallback

; Exports.
global Hal_Boot
global Halx86_GetCurrentCpuBlock
global Halx86_inb
global Halx86_inw
global Halx86_indw
global Halx86_outb
global Halx86_outw
global Halx86_outdw
global Halx86_sti
global Halx86_cli
global Halx86_InitIdt

SECTION .boot

Hal_Boot:
    jmp     Hal_KernelEntry
    
SECTION .text

Halx86_GetCurrentCpuBlock:
    mov     eax, [gs:0x0000]
    ret

Halx86_inb:
    ; Prologue.
    push    ebp
    mov     ebp, esp
    
    ; Body
    xor     eax, eax
    mov     edx, [ebp+8]
    
    in      al, dx
    
    ; Epilogue.
    leave
    ret

Halx86_inw:
    ; Prologue.
    push    ebp
    mov     ebp, esp
    
    ; Body
    xor     eax, eax
    mov     edx, [ebp+8]
    
    in      ax, dx
    
    ; Epilogue.
    leave
    ret

Halx86_indw:
    ; Prologue.
    push    ebp
    mov     ebp, esp
    
    ; Body
    xor     eax, eax
    mov     edx, [ebp+8]
    
    in      eax, dx
    
    ; Epilogue.
    leave
    ret

Halx86_outb:
    ; Prologue.
    push    ebp
    mov     ebp, esp
    
    ; Body
    mov     edx, [ebp+8]
    mov     eax, [ebp+12]
    
    out     dx, al
    
    ; Epilogue.
    leave
    ret

Halx86_outw:
    ; Prologue.
    push    ebp
    mov     ebp, esp
    
    ; Body
    mov     edx, [ebp+8]
    mov     eax, [ebp+12]
    
    out     dx, ax
    
    ; Epilogue.
    leave
    ret

Halx86_outdw:
    ; Prologue.
    push    ebp
    mov     ebp, esp
    
    ; Body
    mov     edx, [ebp+8]
    mov     eax, [ebp+12]
    
    out     dx, eax
    
    ; Epilogue.
    leave
    ret

Halx86_sti:
    sti
    ret

Halx86_cli:
    cli
    ret

HalIsr_LoadEntry:
    ; ecx = ISR address.
    ; edx = [Flags]:[ISR Index]
    mov     ebx, edx
    and     ebx, 0x0000FFFF
    sal     ebx, 3      ; 8 * index.
    add     ebx, HAL_IDT_TABLE
    
    mov     eax, ecx
    and     eax, 0x0000FFFF
    or      eax, (HAL_KERNEL_CS << 16)
    mov     [ebx+0], eax
    
    mov     eax, ecx
    and     eax, 0xFFFF0000
    sar     edx, 16
    and     edx, 0x0000FFFF
    or      eax, edx
    mov     [ebx+4], eax
    
    ret
    
%define HAL_ISR_GATE_FLAGS      0x8F000000  ; Gates clear IF.
%define HAL_ISR_TRAP_FLAGS      0x8E000000  ; Traps do not clear IF.
%define HAL_ISR_SYSCALL_FLAGS   0xEE000000  ; Don't clear IF, allow ring 3.

; Initialize the IDT entries.
Halx86_InitIdt:
    pushad
    
    ; Zero the IDT_TABLE.
    mov     ecx, HAL_IDT_TABLE_LEN
    mov     eax, HAL_IDT_TABLE
Halx86_InitIdt_0:
    mov     byte [eax], 0
    dec     ecx
    jnz     Halx86_InitIdt_0
    
    ; Setup the IDT to point at the table.
    mov     eax, HAL_IDT_TABLE_LEN-1
    mov     [HAL_IDT], ax
    mov     eax, HAL_IDT_TABLE
    mov     [HAL_IDT+2], eax
    
    ; Load up all the IDT entries we care about.
    mov     ecx, HalIsr_0x00
    mov     edx, HAL_ISR_TRAP_FLAGS+0x00
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x01
    mov     edx, HAL_ISR_TRAP_FLAGS+0x01
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x02
    mov     edx, HAL_ISR_TRAP_FLAGS+0x02
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x03
    mov     edx, HAL_ISR_TRAP_FLAGS+0x03
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x04
    mov     edx, HAL_ISR_TRAP_FLAGS+0x04
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x05
    mov     edx, HAL_ISR_TRAP_FLAGS+0x05
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x06
    mov     edx, HAL_ISR_TRAP_FLAGS+0x06
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x07
    mov     edx, HAL_ISR_TRAP_FLAGS+0x07
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x08
    mov     edx, HAL_ISR_TRAP_FLAGS+0x08
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x09
    mov     edx, HAL_ISR_TRAP_FLAGS+0x09
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x0A
    mov     edx, HAL_ISR_TRAP_FLAGS+0x0A
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x0B
    mov     edx, HAL_ISR_TRAP_FLAGS+0x0B
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x0C
    mov     edx, HAL_ISR_TRAP_FLAGS+0x0C
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x0D
    mov     edx, HAL_ISR_TRAP_FLAGS+0x0D
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x0E
    mov     edx, HAL_ISR_TRAP_FLAGS+0x0E
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x10
    mov     edx, HAL_ISR_TRAP_FLAGS+0x10
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x11
    mov     edx, HAL_ISR_TRAP_FLAGS+0x11
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x12
    mov     edx, HAL_ISR_TRAP_FLAGS+0x12
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x13
    mov     edx, HAL_ISR_TRAP_FLAGS+0x13
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x18
    mov     edx, HAL_ISR_TRAP_FLAGS+0x18
    call    HalIsr_LoadEntry
    
    ; These are IRQ interrupts, we will use traps for these as well.
    mov     ecx, HalIsr_0x20
    mov     edx, HAL_ISR_TRAP_FLAGS+0x20
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x21
    mov     edx, HAL_ISR_TRAP_FLAGS+0x21
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x22
    mov     edx, HAL_ISR_TRAP_FLAGS+0x22
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x23
    mov     edx, HAL_ISR_TRAP_FLAGS+0x23
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x24
    mov     edx, HAL_ISR_TRAP_FLAGS+0x24
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x25
    mov     edx, HAL_ISR_TRAP_FLAGS+0x25
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x26
    mov     edx, HAL_ISR_TRAP_FLAGS+0x26
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x27
    mov     edx, HAL_ISR_TRAP_FLAGS+0x27
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x28
    mov     edx, HAL_ISR_TRAP_FLAGS+0x28
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x29
    mov     edx, HAL_ISR_TRAP_FLAGS+0x29
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x2A
    mov     edx, HAL_ISR_TRAP_FLAGS+0x2A
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x2B
    mov     edx, HAL_ISR_TRAP_FLAGS+0x2B
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x2C
    mov     edx, HAL_ISR_TRAP_FLAGS+0x2C
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x2D
    mov     edx, HAL_ISR_TRAP_FLAGS+0x2D
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x2E
    mov     edx, HAL_ISR_TRAP_FLAGS+0x2E
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x2F
    mov     edx, HAL_ISR_TRAP_FLAGS+0x2F
    call    HalIsr_LoadEntry
    
    mov     ecx, HalIsr_0x80
    mov     edx, HAL_ISR_SYSCALL_FLAGS+0x80
    call    HalIsr_LoadEntry
    
    ; Tell the CPU about the table now.
    lidt    [HAL_IDT]
    
    popad
    ret
    
struc HALX86_ISR_STACK_STATE
    .RegEsp     resd 1      ; 0x00
    .RegSs      resd 1      ; 0x04
    .RegDs      resw 1      ; 0x08
    .RegEs      resw 1      ; 0x0A
    .RegFs      resw 1      ; 0x0C
    .RegGs      resw 1      ; 0x0E
    .RegEdi     resd 1      ; 0x10
    .RegEsi     resd 1      ; 0x14
    .RegEbp     resd 1      ; 0x18
    .RegEspX    resd 1      ; 0x1C
    .RegEbx     resd 1      ; 0x20
    .RegEdx     resd 1      ; 0x24
    .RegEcx     resd 1      ; 0x28
    .RegEax     resd 1      ; 0x2C
    .IsrId      resd 1      ; 0x30
    .IsrErrCode resd 1      ; 0x34
    .RegEip     resd 1      ; 0x38
    .RegCs      resd 1      ; 0x3C
    .RegEflags  resd 1      ; 0x40
    .RegEsp_XX  resd 1      ; 0x44
    .RegSs_XX   resd 1      ; 0x48
    .size:
endstruc

; Here's a generic ISR vector, which determines the interrupt number by
; examining the return address.
HalIsr_Generic:
    pushad
    
    mov     ax, gs
    push    ax
    mov     ax, fs
    push    ax
    mov     ax, es
    push    ax
    mov     ax, ds
    push    ax
    
    ; First push the current SS and ESP.
    mov     eax, ss
    and     eax, 0x0000FFFF
    push    eax
    mov     eax, esp
    push    eax
    
    ; At this point, esp points to HALX86_ISR_STACK_STATE.  Save that in ebp.
    mov     ebp, esp
    
    ; Compare the code segments from the interrupt.  If it isn't the kernel CS
    ; then it means we took a privilege switch.  Grab the SS and ESP from the
    ; interrupt stack values.
    mov     eax, [ebp+HALX86_ISR_STACK_STATE.RegCs]
    cmp     ax, HAL_KERNEL_CS
    je      HalIsr_Generic_0
    
    ; This was a privilege change, grab the SS and ESP of the original stack.
    mov     eax, [ebp+HALX86_ISR_STACK_STATE.RegSs_XX]
    and     eax, 0x0000FFFF
    mov     [ebp+HALX86_ISR_STACK_STATE.RegSs], eax
    
    mov     eax, [ebp+HALX86_ISR_STACK_STATE.RegEsp_XX]
    mov     [ebp+HALX86_ISR_STACK_STATE.RegEsp], eax
    
HalIsr_Generic_0:
    ; Now, allocate space for the full HALX86_CONTEXT_RECORD on the stack.
    sub     esp, HALX86_CONTEXT_RECORD.size
    
    ; Copy relevant information from the stack to the context record.
    ; ESP.
    mov     eax, [ebp+HALX86_ISR_STACK_STATE.RegEsp]
    mov     [esp+HALX86_CONTEXT_RECORD.RegEsp], eax
    ; CR3.
    mov     eax, cr3
    mov     [esp+HALX86_CONTEXT_RECORD.RegCr3], eax
    
    ; ESP points to the context record, push it now since it's the first
    ; parameter to Halx86_IsrRootCallback.
    push    esp     ; Param[2]
    
    mov     eax, [ebp+HALX86_ISR_STACK_STATE.IsrErrCode]
    push    eax     ; Param[1]
    
    mov     eax, [ebp+HALX86_ISR_STACK_STATE.IsrId]
    push    eax     ; Param[0]
    
    ; Call the generic HAL C interrupt handler.
    call    Halx86_IsrRootCallback
    
    ; Clear the 3 parameters.
    add     esp, 12
    
    ; If we are switching to a different thread then ESP changes now.  Since
    ; *all* stacks that would be context switchable live in the kernel, we
    ; only need to change the ESP to switch to a different thread context.
    ; This is because usermode switches will enable the task segment switch.
    mov     eax, [esp+HALX86_CONTEXT_RECORD.RegEsp]
    mov     ebx, [ebp+HALX86_ISR_STACK_STATE.RegEsp]
    sub     ebx, eax
    
    ; Note, now ebx = 0 means no change, we will use that later.
    jz      HalIsr_Generic_NoCtxSwitch
    
    ; Set the new stack pointer.
    mov     esp, eax
    
    ; Set the new CR3 (only if we must).
    mov     eax, [esp+HALX86_CONTEXT_RECORD.RegCr3]
    mov     edx, cr3
    cmp     eax, edx
    je      HalIsr_Generic_NoCr3Change
    
    ; This can be costly, so make sure it is necessary.
    mov     cr3, eax
    
HalIsr_Generic_NoCr3Change:

    ; Refresh the TSS since the kernel should have changed it.
    mov     ax, HAL_KERNEL_TSS
    ltr     ax
    
HalIsr_Generic_NoCtxSwitch:
    
    ; Clean up the context record from the stack.
    add     esp, HALX86_CONTEXT_RECORD.size
    
    ; Discard the SS and ESP, we don't need them for restoring.
    add     esp, 8
    
    ; Epilogue
    pop     ax
    mov     ds, ax
    pop     ax
    mov     es, ax
    pop     ax
    mov     fs, ax
    pop     ax
    mov     gs, ax
    
    popad
    
    ; Pop off interrupt number and exception code.
    add     esp, 8
    iret
    
; Here's a TON of ISR vectors!  HalIsr_Generic is the main driver, these are
; just stubs.
; The stack needs to have the error code (or a dummy 0 value) and then the
; interrupt number.  Some interrupts already have the error code pushed onto
; the stack, which makes this even more fun.
HalIsr_0x00:
    push    dword 0
    push    dword 0x00          ; Division By Zero
    jmp     HalIsr_Generic
HalIsr_0x01:
    push    dword 0
    push    dword 0x01          ; Debug
    jmp     HalIsr_Generic
HalIsr_0x02:
    push    dword 0
    push    dword 0x02          ; NMI
    jmp     HalIsr_Generic
HalIsr_0x03:
    push    dword 0
    push    dword 0x03          ; Breakpoint
    jmp     HalIsr_Generic
HalIsr_0x04:
    push    dword 0
    push    dword 0x04          ; Overflow
    jmp     HalIsr_Generic
HalIsr_0x05:
    push    dword 0
    push    dword 0x05          ; Bound-Range
    jmp     HalIsr_Generic
HalIsr_0x06:
    push    dword 0
    push    dword 0x06          ; Invalid Opcode
    jmp     HalIsr_Generic
HalIsr_0x07:
    push    dword 0
    push    dword 0x07          ; Device Not Available
    jmp     HalIsr_Generic
HalIsr_0x08:
    push    dword 0x08          ; Double Fault
    jmp     HalIsr_Generic
HalIsr_0x09:
    push    dword 0
    push    dword 0x09          ; Coprocessor Segment Overrun
    jmp     HalIsr_Generic
HalIsr_0x0A:
    push    dword 0x0A          ; Invalid TSS
    jmp     HalIsr_Generic
HalIsr_0x0B:
    push    dword 0x0B          ; Segment Not Present
    jmp     HalIsr_Generic
HalIsr_0x0C:
    push    dword 0x0C          ; Stack Exception
    jmp     HalIsr_Generic
HalIsr_0x0D:
    push    dword 0x0D          ; General Protection Fault
    jmp     HalIsr_Generic
HalIsr_0x0E:
    push    dword 0x0E          ; Page Fault
    jmp     HalIsr_Generic
HalIsr_0x10:
    push    dword 0
    push    dword 0x10          ; x87 Floating Point Exception Pending
    jmp     HalIsr_Generic
HalIsr_0x11:
    push    dword 0x11          ; Alignment Check Exception
    jmp     HalIsr_Generic
HalIsr_0x12:
    push    dword 0x12          ; Machine Check Exception
    jmp     HalIsr_Generic
HalIsr_0x13:
    push    dword 0x13          ; SIMD Floating Point Exception
    jmp     HalIsr_Generic
HalIsr_0x18:
    push    dword 0
    push    dword 0x14          ; Security Exception
    jmp     HalIsr_Generic
HalIsr_0x20:
    push    dword 0
    push    dword 0x20          ; IRQ 0
    jmp     HalIsr_Generic
HalIsr_0x21:
    push    dword 0
    push    dword 0x21          ; IRQ 1
    jmp     HalIsr_Generic
HalIsr_0x22:
    push    dword 0
    push    dword 0x22          ; IRQ 2
    jmp     HalIsr_Generic
HalIsr_0x23:
    push    dword 0
    push    dword 0x23          ; IRQ 3
    jmp     HalIsr_Generic
HalIsr_0x24:
    push    dword 0
    push    dword 0x24          ; IRQ 4
    jmp     HalIsr_Generic
HalIsr_0x25:
    push    dword 0
    push    dword 0x25          ; IRQ 5
    jmp     HalIsr_Generic
HalIsr_0x26:
    push    dword 0
    push    dword 0x26          ; IRQ 6
    jmp     HalIsr_Generic
HalIsr_0x27:
    push    dword 0
    push    dword 0x27          ; IRQ 7
    jmp     HalIsr_Generic
HalIsr_0x28:
    push    dword 0
    push    dword 0x28          ; IRQ 8
    jmp     HalIsr_Generic
HalIsr_0x29:
    push    dword 0
    push    dword 0x29          ; IRQ 9
    jmp     HalIsr_Generic
HalIsr_0x2A:
    push    dword 0
    push    dword 0x2A          ; IRQ 10
    jmp     HalIsr_Generic
HalIsr_0x2B:
    push    dword 0
    push    dword 0x2B          ; IRQ 11
    jmp     HalIsr_Generic
HalIsr_0x2C:
    push    dword 0
    push    dword 0x2C          ; IRQ 12
    jmp     HalIsr_Generic
HalIsr_0x2D:
    push    dword 0
    push    dword 0x2D          ; IRQ 13
    jmp     HalIsr_Generic
HalIsr_0x2E:
    push    dword 0
    push    dword 0x2E          ; IRQ 14
    jmp     HalIsr_Generic
HalIsr_0x2F:
    push    dword 0
    push    dword 0x2F          ; IRQ 15
    jmp     HalIsr_Generic
HalIsr_0x80:
    push    dword 0
    push    dword 0x80          ; System Service Call


