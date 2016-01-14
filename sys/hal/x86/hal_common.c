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
#include <krn_stdio.h>
#include "hal_common.h"

typedef struct _HAL_CONSOLE_STATE
{
    char* pBuffer;
    uint32_t Width;
    uint32_t Height;
    uint32_t X;
    uint32_t Y;
    uint8_t OutputAttributes;
} HAL_CONSOLE_STATE;

HAL_CONSOLE_STATE g_Hal_ConsoleState = {0};

typedef struct _HAL_CON_PRINTF_CONTEXT
{
    char* pBuffer;
    size_t Offset;
} HAL_CON_PRINTF_CONTEXT;

void
OSCALL
Hal_conprintf_newline(
    )
{
    g_Hal_ConsoleState.Y++;
    
    if (g_Hal_ConsoleState.Y >= g_Hal_ConsoleState.Height)
    {
        uint32_t x;
        uint32_t y;
        uint32_t offset0;
        uint32_t offset1;
        
        for (y = 0; y < g_Hal_ConsoleState.Height - 1; y++)
        {
            for (x = 0; x < g_Hal_ConsoleState.Width; x++)
            {
                offset0 = 2 * (x + (y * g_Hal_ConsoleState.Width));
                offset1 = offset0 + (2 * g_Hal_ConsoleState.Width);
                
                g_Hal_ConsoleState.pBuffer[offset0+0] = g_Hal_ConsoleState.pBuffer[offset1+0];
                g_Hal_ConsoleState.pBuffer[offset0+1] = g_Hal_ConsoleState.pBuffer[offset1+1];
            }
        }
        
        // Now blank the new line.
        y = g_Hal_ConsoleState.Height - 1;
        
        for (x = 0; x < g_Hal_ConsoleState.Width; x++)
        {
            offset0 = 2 * (x + (y * g_Hal_ConsoleState.Width));
            
            g_Hal_ConsoleState.pBuffer[offset0+0] = ' ';
            g_Hal_ConsoleState.pBuffer[offset0+1] = g_Hal_ConsoleState.OutputAttributes;
        }
        
        g_Hal_ConsoleState.Y = g_Hal_ConsoleState.Height - 1;
    }
}

void
OSCALL
Hal_conprintfWriter(
    void* pContext,
    char Character
    )
{
    uint32_t offset;
    
    offset = 2 * (g_Hal_ConsoleState.X + (g_Hal_ConsoleState.Y * g_Hal_ConsoleState.Width));
    
    if (Character == '\n')
    {
        Hal_conprintf_newline();
        g_Hal_ConsoleState.X = 0;
    }
    else
    {
        g_Hal_ConsoleState.pBuffer[offset+0] = Character;
        g_Hal_ConsoleState.pBuffer[offset+1] = g_Hal_ConsoleState.OutputAttributes;
        
        g_Hal_ConsoleState.X++;
        
        if (g_Hal_ConsoleState.X >= g_Hal_ConsoleState.Width)
        {
            Hal_conprintf_newline();
            g_Hal_ConsoleState.X = 0;
        }
    }
}

// 
// External functions.
//
void
OSCALL
Hal_ConsoleInit(
    char* pBuffer,
    uint32_t Width,
    uint32_t Height
    )
{
    g_Hal_ConsoleState.pBuffer = pBuffer;
    g_Hal_ConsoleState.Width = Width;
    g_Hal_ConsoleState.Height = Height;
    
    g_Hal_ConsoleState.X = 0;
    g_Hal_ConsoleState.Y = 0;
    
    g_Hal_ConsoleState.OutputAttributes = 0x0F;
    
    uint32_t x;
    uint32_t y;
    uint32_t offset;
    
    // Blank the console.
    for (y = 0; y < g_Hal_ConsoleState.Height; y++)
    {
        for (x = 0; x < g_Hal_ConsoleState.Width; x++)
        {
            offset = 2 * (x + (y * g_Hal_ConsoleState.Width));
            
            g_Hal_ConsoleState.pBuffer[offset+0] = ' ';
            g_Hal_ConsoleState.pBuffer[offset+1] = g_Hal_ConsoleState.OutputAttributes;
        }
    }
}

int
OSCALL
Hal_conprintf(
    const char* pFormat,
    ...
    )
{
    va_list args;
    int result;
    
    va_start(args, pFormat);
    
    result = Krn_vsnprintf_func(
            Hal_conprintfWriter,
            NULL,
            1024*1024,
            pFormat,
            args);
    
    va_end(args);
    
    return result;
}


