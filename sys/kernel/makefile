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
# Kernel makefile.
#
# For the assembly tool chain, assume mingw with nasm installed in the bin
# directory.  User should have a mingw environment variable.
#

ROOT_BASE=..

include $(ROOT_BASE)/common/defaults.mk

OUTBASE=$(ROOT_BASE)/build/$(BUILD_ARCH)
OUTDIR=$(OUTBASE)/kernel

LD_FLAGS=--script ldscript_$(BUILD_ARCH) -nostdlib --sysroot=$(SYS_ROOT)

OBJ_FILES= \
	$(OUTDIR)/krn_base.o \
	$(OUTDIR)/krn_main.o \
	$(OUTDIR)/krn_mem.o \
	$(OUTDIR)/krn_stdio.o \

all: kernel_image

publish: all

$(OUTDIR):
	mkdir -p $(OUTDIR)

hal_comp:
	cd ../hal && $(MAKE)

kernel_image: $(OUTDIR) hal_comp $(OUTDIR)/kernel.bin
	cat $(OUTBASE)/hal/hal_pre.bin $(OUTDIR)/kernel.bin > $(OUTDIR)/kernel.img

$(OUTDIR)/kernel.bin: $(OUTDIR) $(OUTDIR)/kernel.elf
	objcopy -O binary $(OUTDIR)/kernel.elf $(OUTDIR)/kernel.bin
	
$(OUTDIR)/kernel.elf: $(OUTDIR) $(OBJ_FILES)
	$(LD) $(LD_FLAGS) \
		-Map $(OUTDIR)/kernel.map \
		-o $(OUTDIR)/kernel.elf \
		$(OUTBASE)/hal/*.o $(OBJ_FILES) \
		$(C_LIBGCC)

include $(ROOT_BASE)/common/gcc_c.mk
	
clean:
	rm -f $(CLEAN_SPEC)
	cd $(OUTDIR) && rm -f $(CLEAN_SPEC)
	cd ../hal && $(MAKE) clean

