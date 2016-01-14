# DarkOs #
Jonathan Ward

## Introduction ##
DarkOs was started to provide first hand experience in operating system
implementation.  Usually, the best way to truly understand the why of a
problem or system is to implement it yourself.  Only then will all the
trade-offs, advantages, and disadvantages become known and understood.

With this in mind, DarkOs should provide a decent examination into the
difficulties of creating a multi-tasking operating system.

## Build System and Tools ##
Since we are building an operating system from the ground up, it's not wise
to use tools which target an existing operating system such as Linux, Windows, or
OSX.  Here is a list of tools required:
- A posix environment -- I use cygwin on Windows.
- A version of binutils capable of targeting i686-elf files and having a sysroot specified.
- A gcc cross compiler for i686-elf files.
- NASM, because I find GNU assembler syntax horribly annoying.

The sources and NASM for the current project binutils and gcc can be found here:
 http://darkautomata.com/DarkOs_Build_Sources_2016_01_14.zip

You probably also want a good (or a few...) x86 emulators.  I use VirtualBox
because of it's nifty debugging capabilities and it's speed.  It boots from a
VHD so updating the disk image is relatively painless.

## Conventions ##
The code is split into the hardware abstraction layer (HAL) and the kernel.

## Booting ##
The current incarnation of DarkOs utilizes the GRUB boot loader.  It includes
a GRUB Multi-Boot 1.0 header, however it's currently being booted from GRUB 2
and I could see adding a GRUB 2 header in the near future.

The first incarnation of the OS included it's own boot loader.  There are multiple
issues with this that resulted in a rocky start.  First, boot loaders need
to do a surprising number of pretty boring things.  An Master Boot Record or
Volume Boot Record is required.  Most existing operating systems seems to have
an idea of what an MBR and VBR looks like.  For example, they are embedded in
format utilities in windows.  The first implementation of a custom boot loader
had rabbit-holed into a custom FAT32 format utility (and thus, parsing and
generating all the FAT32 data structures).  While certainly interesting, this
had deviated quite a bit from the original goal of the project.

As such, GRUB is used.  GRUB gets us to a spot where we can (barely) execute
protected mode code.  It also nicely sets up the VBE frame buffer if requested
and provides various BIOS created data blocks to the OS.  All this is pretty
useful and writing the code to do so is again, not the goal of this project.
As much as possible, calling BIOS interrupt support routines should be avoided.

Onward then, to how we actually boot.  GRUB loads our Hardware Abstraction Layer (HAL)
stubs and the kernel as a single flat file.  While ELF will be used for the
actual DarkOs image file format, the kernel loads as a flat file.  The layout
of the "kernel" image file is:

Kernel Image File Layout
    Component          File Offset    Size       Load Base
    ----------------------------------------------------------
    HAL Upper Stub     0x00000000     8KB        0x01000000
    Kernel             0x00004000     -          0x01004000

## Hardware Abstraction Layer (HAL) Common ##
Partially implemented, need docs.

## Hardware Abstraction Layer (HAL) x86 ##
Partially implemented, need docs.

## CPU Context Block ##
A structure specific to each CPU in the system.  Referenced via [gs:0x0000].

Current CPU Context Block Format
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

## Thread Context Block ##
TODO

## Process Context Block ##
TODO

## Segments ##
Segments indexes are configured globally for the system.

Current System Segments
    0x00 - Unused
    0x08 - 32-bit Kernel Code
    0x10 - 32-bit Kernel Data
    0x18 - CPU Data segment
    0x20 - Task Segment
    0x28 - 32-bit User Code
    0x30 - 32-bit User Data

## Paging ##
The lower 4 MB is identity mapped.  The lower 32 MB is also mapped to 0x80000000.
TODO: Discuss kernel vs. user page directory layout.
TODO: Discuss kernel PDEs always present (wasting 2MB of RAM, but for good reason).

## Interrupts ##
Partially implemented, see Halx86_IsrRootCallback.

## Context Switching ##
Not yet implemented, although close.  Design is taking shape now.

## Wait Blocks ##
Not yet implemented.  Need decent OS data structures (linked list, stack, etc.) which
have yet to be tested.



