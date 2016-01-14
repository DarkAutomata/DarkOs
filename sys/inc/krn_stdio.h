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

// Kernel mode standard I/O functionality.

#include <krn_base.h>

#ifndef __KRN_STDIO_H__
#define __KRN_STDIO_H__

// 
// For calling the lower-level printf functions, this will get called for each
// character to be printed.  Internally this is used to print to a buffer, or
// to a terminal, etc.
// 
typedef void (OSCALL *KRN_PRINTF_CHAR_WRITER_FUNC)(
    void* pContext,
    char Character
    );

int
OSCALL
Krn_vsnprintf(
    char* pDest,
    size_t DestSize,
    const char* pFormat,
    va_list args
    );

int
OSCALL
Krn_vsnprintf_func(
    KRN_PRINTF_CHAR_WRITER_FUNC pOutFunc,
    void* pOutFuncContext,
    size_t MaxSize,
    const char* pFormat,
    va_list args
    );

#endif // __KSTDIO_H__


