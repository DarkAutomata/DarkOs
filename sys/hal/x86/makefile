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
# x86 HAL routines.
#
# For the assembly tool chain, assume mingw with nasm installed in the bin
# directory.  User should have a mingw environment variable.
#

ROOT_BASE=../..

include $(ROOT_BASE)/common/defaults.mk

OUTBASE=$(ROOT_BASE)/build/$(BUILD_ARCH)
OUTDIR=$(OUTBASE)/hal

OBJ_FILES=$(OUTDIR)/hal_asm.o \
		  $(OUTDIR)/hal_main.o \
		  $(OUTDIR)/hal_common.o \

all: $(OUTDIR)/hal_pre.bin $(OBJ_FILES)

publish: all

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/hal_pre.bin: $(OUTDIR) $(OUTDIR)/hal_ustub.bin
	cp $(OUTDIR)/hal_ustub.bin $(OUTDIR)/hal_pre.bin

$(OUTDIR)/hal_ustub.bin: $(OUTDIR) hal_ustub.asm
	nasm -f bin -o $(OUTDIR)/hal_ustub.bin -l $(OUTDIR)/hal_ustub.bin.lst hal_ustub.asm

$(OUTDIR)/hal_asm.o: $(OUTDIR) hal_asm.asm
	nasm -f elf32 -o $@ -l $@.lst hal_asm.asm

include $(ROOT_BASE)/common/gcc_c.mk

clean:
	rm -f $(CLEAN_SPEC)
	cd $(OUTDIR) && rm -f $(CLEAN_SPEC) *.a

