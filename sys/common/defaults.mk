# Copyright (c) 2016, Jonathan Ward
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# 
# Default make settings.
#

SYS_ROOT=/cross/i686-elf

INCLUDE=-I$(ROOT_BASE)/inc -I$(ROOT_BASE)/hal

# For LLVM builds
CC=$(SYS_ROOT)/bin/i686-elf-gcc
C_FLAGS=-mabi=sysv -std=gnu99 -ffreestanding -fno-builtin -Wall -Werror --sysroot=$(SYS_ROOT) -gstabs
C_LISTING=-Wa,-ahlms
C_LIBGCC=$(SYS_ROOT)/lib/gcc/i686-elf/4.8.5/libgcc.a

AR=$(SYS_ROOT)/bin/i686-elf-ar
LD=$(SYS_ROOT)/bin/i686-elf-ld

CLEAN_SPEC=*.o *.bin *.lst *.elf *.exe *.map *.img *~ *.a
BUILD_ARCH=x86

