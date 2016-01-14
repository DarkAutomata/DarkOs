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

#include <krn_base.h>

// Error reporting data.
struct
{
    uint32_t ErrorCode;
    const char* pSourceFile;
    int SourceLine;
    const void* pParam;
} g_Krn_ErrData = {0};

void
OSCALL
Krn_Main(
    const char* pArgs
    )
{
    while(1);
}

void
OSCALL
Krn_ErrFunc(
    uint32_t ErrorCode,
    const char* pSourceFile,
    int SourceLine,
    const void* pParam
    )
{
    g_Krn_ErrData.ErrorCode = ErrorCode;
    g_Krn_ErrData.pSourceFile = pSourceFile;
    g_Krn_ErrData.SourceLine = SourceLine;
    g_Krn_ErrData.pParam = pParam;
    
    if (ErrorCode & KRN_ERR_FATAL_MASK)
    {
        while(1);
    }
}


