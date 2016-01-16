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

#ifndef __KRN_MEM_H__
#define __KRN_MEM_H__

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

#define KRN_MEM_FLAGS_READ      0x00000001
#define KRN_MEM_FLAGS_WRITE     0x00000002
#define KRN_MEM_FLAGS_EXEC      0x00000004

#define KRN_MEM_OPTS_CONTIG     0x80000000

typedef struct _KRN_MEM_PAGE_DB_ENTRY KRN_MEM_PAGE_DB_ENTRY;

#define KRN_MEM_PAGE_0_FLAG_USED    0x80000000
#define KRN_MEM_PAGE_0_FLAG_SPEC    0x40000000
#define KRN_MEM_PAGE_0_FLAG_IO      0x20000000
#define KRN_MEM_PAGE_0_MASK_KRN     0x00FFFFFF
#define KRN_MEM_PAGE_1_MASK_USR     0x00FFFFFF

struct _KRN_MEM_PAGE_DB_ENTRY
{
    uint32_t PageFlags0;
    uint32_t PageFlags1;
};


#endif // __KRN_MEM_H__


