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
#include <krn_stdio.h>

// 
// Private constants.
// 

#define KRN_PRINTF_FLAG_MASK        0x000000FF
#define KRN_PRINTF_FLAG_MINUS       0x00000001      // '-'
#define KRN_PRINTF_FLAG_PLUS        0x00000002      // '+'
#define KRN_PRINTF_FLAG_SPACE       0x00000004      // ' '
#define KRN_PRINTF_FLAG_ZERO        0x00000008      // '0'
#define KRN_PRINTF_FLAG_HASH        0x00000010      // '#'
#define KRN_PRINTF_SIZE_MASK        0x0000FF00
#define KRN_PRINTF_SIZE_h           0x00000100      // 'h'
#define KRN_PRINTF_SIZE_l           0x00000200      // 'l'
#define KRN_PRINTF_SIZE_ll          0x00000300      // 'll'
#define KRN_PRINTF_SIZE_w           0x00000400      // 'w'
#define KRN_PRINTF_SIZE_I           0x00000500      // 'I'
#define KRN_PRINTF_SIZE_I32         0x00000600      // 'I32'
#define KRN_PRINTF_SIZE_I64         0x00000700      // 'I64'
#define KRN_PRINTF_TYPE_MASK        0x00FF0000
#define KRN_PRINTF_TYPE_c           0x00010000      // 'c'
#define KRN_PRINTF_TYPE_C           0x00020000      // 'C'
#define KRN_PRINTF_TYPE_d           0x00030000      // 'd'
#define KRN_PRINTF_TYPE_i           0x00040000      // 'i'
#define KRN_PRINTF_TYPE_o           0x00050000      // 'o'
#define KRN_PRINTF_TYPE_u           0x00060000      // 'u'
#define KRN_PRINTF_TYPE_x           0x00070000      // 'x'
#define KRN_PRINTF_TYPE_X           0x00080000      // 'X'
#define KRN_PRINTF_TYPE_e           0x00090000      // 'e'
#define KRN_PRINTF_TYPE_E           0x000A0000      // 'E'
#define KRN_PRINTF_TYPE_g           0x000B0000      // 'g'
#define KRN_PRINTF_TYPE_G           0x000C0000      // 'G'
#define KRN_PRINTF_TYPE_a           0x000D0000      // 'a'
#define KRN_PRINTF_TYPE_A           0x000E0000      // 'A'
#define KRN_PRINTF_TYPE_n           0x000F0000      // 'n'
#define KRN_PRINTF_TYPE_p           0x00100000      // 'p'
#define KRN_PRINTF_TYPE_s           0x00110000      // 's'
#define KRN_PRINTF_TYPE_S           0x00120000      // 'S'

// 
// These are used to indicate how far we have progressed through the parsing
// chunks.  A simple greater than comparison can be used to know if we are
// getting out-of-order identifiers.
// 
#define KRN_PRINTF_PROG_MASK        0xFF000000
#define KRN_PRINTF_PROG_FLAGS       0x01000000
#define KRN_PRINTF_PROG_WIDTH       0x02000000
#define KRN_PRINTF_PROG_PREC        0x04000000
#define KRN_PRINTF_PROG_SIZE        0x08000000
#define KRN_PRINTF_PROG_TYPE        0x10000000

#define KRN_PRINTF_WIDTH_DEFAULT    0x80000000      // Not specified.
#define KRN_PRINTF_WIDTH_ARG        0x80000001      // Using '*'
#define KRN_PRINTF_WIDTH_MASK       0x7FFFFFFF

#define KRN_PRINTF_CTYPE_SIGNED     0x80000000      // Indicates signed type.
#define KRN_PRINTF_CTYPE_CHAR       0x80000001      // char
#define KRN_PRINTF_CTYPE_SHORT      0x80000002      // short
#define KRN_PRINTF_CTYPE_USHORT     0x00000002      // unsigned short
#define KRN_PRINTF_CTYPE_INT        0x80000003      // int
#define KRN_PRINTF_CTYPE_UINT       0x00000003      // unsigned int
#define KRN_PRINTF_CTYPE_LONG       0x80000004      // long
#define KRN_PRINTF_CTYPE_ULONG      0x00000004      // unsigned long
#define KRN_PRINTF_CTYPE_LLONG      0x80000005      // long long
#define KRN_PRINTF_CTYPE_ULLONG     0x00000005      // unsigned long long
#define KRN_PRINTF_CTYPE_FLOAT      0x80000006      // float
#define KRN_PRINTF_CTYPE_DOUBLE     0x80000007      // double
#define KRN_PRINTF_CTYPE_PTR        0x00000008      // pointer value
#define KRN_PRINTF_CTYPE_SIZE       0x00000009      // size_t
#define KRN_PRINTF_CTYPE_PTRDIFF    0x8000000A      // ptrdiff_t
#define KRN_PRINTF_CTYPE_I32        0x8000000B      // int32_t
#define KRN_PRINTF_CTYPE_U32        0x0000000B      // uint32_t
#define KRN_PRINTF_CTYPE_I64        0x8000000C      // int64_t
#define KRN_PRINTF_CTYPE_U64        0x0000000C      // uint64_t

typedef struct _KRN_PRINTF_PARSE_ENTRY
{
    const char* pId;                // C-string ID.
    uint32_t ParseFlag;             // The parse flag bit to set.
    uint32_t ProgBits;              // Which parse grouping.
    uint32_t ExclusiveBits;         // Exclude mask (can't have 2 types, etc.)
} KRN_PRINTF_PARSE_ENTRY;

typedef struct _KRN_PRINTF_VAL_SPEC
{
    uint32_t Flags;
    uint32_t Width;
    uint32_t Precision;
    char TmpBuffer[64];
} KRN_PRINTF_VAL_SPEC;


// 
// Helper functions.
// 

static size_t
Krn_PrintfDecodeNumber(
    const char* pFormat,
    uint32_t* pValue
    );

static size_t
Krn_PrintfDecodeSpec(
    const char* pFormat,
    KRN_PRINTF_VAL_SPEC* pSpec
    );

static size_t
Krn_PrintfOutputSpec(
    KRN_PRINTF_CHAR_WRITER_FUNC pOutFunc,
    void* pOutFuncContext,
    size_t MaxChars,
    KRN_PRINTF_VAL_SPEC* pSpec,
    va_list* pArgList
    );

typedef struct _KRN_PRINTF_DEF_STRING_WRITER_CONTEXT
{
    char* pBuffer;
    size_t Offset;
} KRN_PRINTF_DEF_STRING_WRITER_CONTEXT;

void
OSCALL
Krn_PrintfDefaultStringWriter(
    void* pContext,
    char Character
    );

// 
// Function implementations.
//

static size_t
Krn_PrintfDecodeNumber(
    const char* pFormat,
    uint32_t* pValue
    )
{
    size_t charsUsed = 0;
    
    *pValue = 0;
    
    while ((pFormat[charsUsed] >= '0') &&
           (pFormat[charsUsed] <= '9') &&
           (*pValue < KRN_PRINTF_WIDTH_MASK))
    {
        *pValue *= 10;
        *pValue += (pFormat[charsUsed] - '0');
        
        charsUsed++;
    }
    
    if (*pValue >= KRN_PRINTF_WIDTH_MASK)
    {
        Krn_Err(KRN_ERR_INV_PRINTF_FORMAT, NULL);
        *pValue = 0;
        charsUsed = 0;
    }
    
    return charsUsed;
}

static size_t
Krn_PrintfDecodeSpec(
    const char* pFormat,
    KRN_PRINTF_VAL_SPEC* pSpec
    )
{
    size_t charsUsed;
    int i;
    const KRN_PRINTF_PARSE_ENTRY* pBest;
    int bestCharCount;
    uint32_t progressBits;
    
    const KRN_PRINTF_PARSE_ENTRY parseTable[] =
    {
        { "-",      KRN_PRINTF_FLAG_MINUS,  KRN_PRINTF_PROG_FLAGS,  0 },
        { "+",      KRN_PRINTF_FLAG_PLUS,   KRN_PRINTF_PROG_FLAGS,  0 },
        { " ",      KRN_PRINTF_FLAG_SPACE,  KRN_PRINTF_PROG_FLAGS,  0 },
        { "0",      KRN_PRINTF_FLAG_ZERO,   KRN_PRINTF_PROG_FLAGS,  0 },
        { "#",      KRN_PRINTF_FLAG_HASH,   KRN_PRINTF_PROG_FLAGS,  0 },
        
        { "h",      KRN_PRINTF_SIZE_h,      KRN_PRINTF_PROG_SIZE,   KRN_PRINTF_SIZE_MASK },
        { "l",      KRN_PRINTF_SIZE_l,      KRN_PRINTF_PROG_SIZE,   KRN_PRINTF_SIZE_MASK },
        { "ll",     KRN_PRINTF_SIZE_ll,     KRN_PRINTF_PROG_SIZE,   KRN_PRINTF_SIZE_MASK },
        { "w",      KRN_PRINTF_SIZE_w,      KRN_PRINTF_PROG_SIZE,   KRN_PRINTF_SIZE_MASK },
        { "I",      KRN_PRINTF_SIZE_I,      KRN_PRINTF_PROG_SIZE,   KRN_PRINTF_SIZE_MASK },
        { "I32",    KRN_PRINTF_SIZE_I32,    KRN_PRINTF_PROG_SIZE,   KRN_PRINTF_SIZE_MASK },
        { "I64",    KRN_PRINTF_SIZE_I64,    KRN_PRINTF_PROG_SIZE,   KRN_PRINTF_SIZE_MASK },
        
        { "c",      KRN_PRINTF_TYPE_c,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "C",      KRN_PRINTF_TYPE_C,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "d",      KRN_PRINTF_TYPE_d,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "i",      KRN_PRINTF_TYPE_i,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "o",      KRN_PRINTF_TYPE_o,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "u",      KRN_PRINTF_TYPE_u,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "x",      KRN_PRINTF_TYPE_x,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "X",      KRN_PRINTF_TYPE_X,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        // { "e",      KRN_PRINTF_TYPE_e,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        // { "E",      KRN_PRINTF_TYPE_E,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        // { "g",      KRN_PRINTF_TYPE_g,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        // { "G",      KRN_PRINTF_TYPE_G,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        // { "a",      KRN_PRINTF_TYPE_a,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        // { "A",      KRN_PRINTF_TYPE_A,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "n",      KRN_PRINTF_TYPE_n,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "p",      KRN_PRINTF_TYPE_p,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "s",      KRN_PRINTF_TYPE_s,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
        { "S",      KRN_PRINTF_TYPE_S,      KRN_PRINTF_PROG_TYPE,   KRN_PRINTF_TYPE_MASK },
    };
    
    charsUsed = 0;
    
    // Reset the spec structure.
    pSpec->Flags = 0;
    pSpec->Width = KRN_PRINTF_WIDTH_DEFAULT;
    pSpec->Precision = KRN_PRINTF_WIDTH_DEFAULT;
    
    while (0 == (pSpec->Flags & KRN_PRINTF_TYPE_MASK))
    {
        // Get the current progress bits so we know what to look for now.
        progressBits = pSpec->Flags & KRN_PRINTF_PROG_MASK;
        pBest = NULL;
        bestCharCount = 0;
        
        // Search the table above for the best parse match.
        for (i = 0; i < _countof(parseTable); i++)
        {
            if ((parseTable[i].ProgBits >= progressBits) &&
                ((parseTable[i].ExclusiveBits & pSpec->Flags) == 0))
            {
                // This is an entry we can consider.
                uint32_t j = 0;
                
                while ((parseTable[i].pId[j]) &&
                       (pFormat[charsUsed + j] == parseTable[i].pId[j]))
                {
                    j++;
                }
                
                // j is the match count.
                if (j > bestCharCount)
                {
                    // New best match.
                    pBest = &parseTable[i];
                    bestCharCount = j;
                }
            }
        }
        
        // If we found a match then move on.
        if (pBest)
        {
            pSpec->Flags |= pBest->ParseFlag;
            pSpec->Flags |= pBest->ProgBits;
            charsUsed += bestCharCount;
        }
        else if ((KRN_PRINTF_PROG_WIDTH >= progressBits) &&
                 (pFormat[charsUsed] >= '0') &&
                 (pFormat[charsUsed] <= '9'))
        {
            // Width.
            size_t charsMatched;
            
            pSpec->Width = 0;
            
            charsMatched = Krn_PrintfDecodeNumber(&pFormat[charsUsed], &pSpec->Width);
            if (charsMatched == 0)
            {
                Krn_Err(KRN_ERR_INV_PRINTF_FORMAT, NULL);
                return 0;
            }
            
            pSpec->Flags |= KRN_PRINTF_PROG_WIDTH;
            charsUsed += charsMatched;
        }
        else if ((KRN_PRINTF_PROG_WIDTH >= progressBits) &&
                 (pFormat[charsUsed] == '*'))
        {
            // Width.
            pSpec->Width = KRN_PRINTF_WIDTH_ARG;
            pSpec->Flags |= KRN_PRINTF_PROG_WIDTH;
            charsUsed += 1;
        }
        else if ((KRN_PRINTF_PROG_WIDTH >= progressBits) &&
                 (pFormat[charsUsed] == '.'))
        {
            // Precision.
            pSpec->Precision = 0;
            
            charsUsed++;
            
            if ((pFormat[charsUsed] >= '0') &&
                (pFormat[charsUsed] <= '9'))
            {
                size_t charsMatched;
                
                charsMatched = Krn_PrintfDecodeNumber(&pFormat[charsUsed], &pSpec->Precision);
                if (charsMatched == 0)
                {
                    Krn_Err(KRN_ERR_INV_PRINTF_FORMAT, NULL);
                    return 0;
                }
                
                charsUsed += charsMatched;
            }
            else if (pFormat[charsUsed] == '*')
            {
                pSpec->Precision = KRN_PRINTF_WIDTH_ARG;
                charsUsed++;
            }
            else
            {
                pSpec->Precision = KRN_PRINTF_WIDTH_DEFAULT;
            }
            
            pSpec->Flags |= KRN_PRINTF_PROG_PREC;
        }
        else
        {
            Krn_Err(KRN_ERR_INV_PRINTF_FORMAT, NULL);
            return 0;
        }
    }
    
    // Krn_Err(KRN_ERR_ASSERT_FAILED, pSpec);
            
    return charsUsed;
}

static size_t
Krn_PrintfOutputSpec(
    KRN_PRINTF_CHAR_WRITER_FUNC pOutFunc,
    void* pOutFuncContext,
    size_t MaxChars,
    KRN_PRINTF_VAL_SPEC* pSpec,
    va_list* pArgList
    )
{
    size_t charsWritten = 0;
    uint32_t typeId = 0;
    uint32_t sizeId = 0;
    uint32_t dataType = 0;  // One of KRN_PRINTF_CTYPE_*
    uint32_t divBase = 0;
    
    const char lowerHexTable[] = 
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
    };
    const char upperHexTable[] = 
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    };
    
    union
    {
        void* v_ptr;
        int64_t v_i;
        uint64_t v_u;
        
        // TODO: Handle floats.
    } value;
    
    // Handle some mutually exclusive flag conflicts.
    if ((pSpec->Flags & KRN_PRINTF_FLAG_MINUS) &&
        (pSpec->Flags & KRN_PRINTF_FLAG_ZERO))
    {
        // 0 is ignored if - is specified.
        pSpec->Flags &= ~(KRN_PRINTF_FLAG_ZERO);
    }
    
    if ((pSpec->Flags & KRN_PRINTF_FLAG_PLUS) &&
        (pSpec->Flags & KRN_PRINTF_FLAG_SPACE))
    {
        // space is ignored if + specified.
        pSpec->Flags &= ~(KRN_PRINTF_FLAG_SPACE);
    }
    
    dataType = 0xFFFFFFFF;
    
    typeId = pSpec->Flags & KRN_PRINTF_TYPE_MASK;
    sizeId = pSpec->Flags & KRN_PRINTF_SIZE_MASK;
    
    // Set the proper base, this doesn't change.
    divBase = 10;
    
    if (typeId == KRN_PRINTF_TYPE_o)
    {
        divBase = 8;
    }
    else if ((typeId == KRN_PRINTF_TYPE_x) ||
             (typeId == KRN_PRINTF_TYPE_X) ||
             (typeId == KRN_PRINTF_TYPE_p))
    {
        divBase = 16;
    }
    
    // First, determine type without the size modifier.  Then we will adjust
    // for valid modifiers.
    if (typeId == KRN_PRINTF_TYPE_C)
    {
        dataType = KRN_PRINTF_CTYPE_CHAR;
    }
    else if ((typeId == KRN_PRINTF_TYPE_d) ||
             (typeId == KRN_PRINTF_TYPE_i))
    {
        dataType = KRN_PRINTF_CTYPE_INT;
    }
    else if ((typeId == KRN_PRINTF_TYPE_o) ||
             (typeId == KRN_PRINTF_TYPE_u) ||
             (typeId == KRN_PRINTF_TYPE_x) ||
             (typeId == KRN_PRINTF_TYPE_X))
    {
        dataType = KRN_PRINTF_CTYPE_UINT;
    }
    else if ((typeId == KRN_PRINTF_TYPE_n) ||
             (typeId == KRN_PRINTF_TYPE_p) ||
             (typeId == KRN_PRINTF_TYPE_s) ||
             (typeId == KRN_PRINTF_TYPE_S))
    {
        dataType = KRN_PRINTF_CTYPE_PTR;
    }
    
    if (sizeId == KRN_PRINTF_SIZE_h)
    {
        if (typeId == KRN_PRINTF_TYPE_C)
        {
            dataType = KRN_PRINTF_CTYPE_CHAR;
        }
        else if ((typeId == KRN_PRINTF_TYPE_d) ||
                 (typeId == KRN_PRINTF_TYPE_i))
        {
            dataType = KRN_PRINTF_CTYPE_SHORT;
        }
        else if ((typeId == KRN_PRINTF_TYPE_o) ||
                 (typeId == KRN_PRINTF_TYPE_u) ||
                 (typeId == KRN_PRINTF_TYPE_x) ||
                 (typeId == KRN_PRINTF_TYPE_X))
        {
            dataType = KRN_PRINTF_CTYPE_USHORT;
        }
    }
    else if (sizeId == KRN_PRINTF_SIZE_l)
    {
        if (typeId == KRN_PRINTF_TYPE_C)
        {
            dataType = KRN_PRINTF_CTYPE_SHORT;
        }
        else if ((typeId == KRN_PRINTF_TYPE_d) ||
                 (typeId == KRN_PRINTF_TYPE_i))
        {
            dataType = KRN_PRINTF_CTYPE_LONG;
        }
        else if ((typeId == KRN_PRINTF_TYPE_o) ||
                 (typeId == KRN_PRINTF_TYPE_u) ||
                 (typeId == KRN_PRINTF_TYPE_x) ||
                 (typeId == KRN_PRINTF_TYPE_X))
        {
            dataType = KRN_PRINTF_CTYPE_ULONG;
        }
    }
    else if (sizeId == KRN_PRINTF_SIZE_ll)
    {
        if ((typeId == KRN_PRINTF_TYPE_d) ||
            (typeId == KRN_PRINTF_TYPE_i))
        {
            dataType = KRN_PRINTF_CTYPE_LLONG;
        }
        else if ((typeId == KRN_PRINTF_TYPE_o) ||
                 (typeId == KRN_PRINTF_TYPE_u) ||
                 (typeId == KRN_PRINTF_TYPE_x) ||
                 (typeId == KRN_PRINTF_TYPE_X))
        {
            dataType = KRN_PRINTF_CTYPE_ULLONG;
        }
    }
    else if (sizeId == KRN_PRINTF_SIZE_w)
    {
        if (typeId == KRN_PRINTF_TYPE_C)
        {
            dataType = KRN_PRINTF_CTYPE_SHORT;
        }
    }
    else if (sizeId == KRN_PRINTF_SIZE_I)
    {
        if ((typeId == KRN_PRINTF_TYPE_d) ||
            (typeId == KRN_PRINTF_TYPE_i))
        {
            dataType = KRN_PRINTF_CTYPE_PTRDIFF;
        }
        else if ((typeId == KRN_PRINTF_TYPE_o) ||
                 (typeId == KRN_PRINTF_TYPE_u) ||
                 (typeId == KRN_PRINTF_TYPE_x) ||
                 (typeId == KRN_PRINTF_TYPE_X))
        {
            dataType = KRN_PRINTF_CTYPE_SIZE;
        }
    }
    else if (sizeId == KRN_PRINTF_SIZE_I32)
    {
        if ((typeId == KRN_PRINTF_TYPE_d) ||
            (typeId == KRN_PRINTF_TYPE_i))
        {
            dataType = KRN_PRINTF_CTYPE_I32;
        }
        else if ((typeId == KRN_PRINTF_TYPE_o) ||
                 (typeId == KRN_PRINTF_TYPE_u) ||
                 (typeId == KRN_PRINTF_TYPE_x) ||
                 (typeId == KRN_PRINTF_TYPE_X))
        {
            dataType = KRN_PRINTF_CTYPE_U32;
        }
    }
    else if (sizeId == KRN_PRINTF_SIZE_I64)
    {
        if ((typeId == KRN_PRINTF_TYPE_d) ||
            (typeId == KRN_PRINTF_TYPE_i))
        {
            dataType = KRN_PRINTF_CTYPE_I64;
        }
        else if ((typeId == KRN_PRINTF_TYPE_o) ||
                 (typeId == KRN_PRINTF_TYPE_u) ||
                 (typeId == KRN_PRINTF_TYPE_x) ||
                 (typeId == KRN_PRINTF_TYPE_X))
        {
            dataType = KRN_PRINTF_CTYPE_U64;
        }
    }
    
    // Initialize value to 0.
    memset(&value, 0, sizeof(value));
    
    // Now read the type from the arg list.
    // NOTE: char and short are promoted to int by the caller of any va_args
    // functions.
    if (dataType == KRN_PRINTF_CTYPE_CHAR)
    {
        value.v_i = va_arg(*pArgList, int);
    }
    else if (dataType == KRN_PRINTF_CTYPE_SHORT)
    {
        value.v_i = va_arg(*pArgList, int);
    }
    else if (dataType == KRN_PRINTF_CTYPE_USHORT)
    {
        value.v_u = va_arg(*pArgList, unsigned int);
    }
    else if (dataType == KRN_PRINTF_CTYPE_INT)
    {
        value.v_i = va_arg(*pArgList, int);
    }
    else if (dataType == KRN_PRINTF_CTYPE_UINT)
    {
        value.v_u = va_arg(*pArgList, unsigned int);
    }
    else if (dataType == KRN_PRINTF_CTYPE_LONG)
    {
        value.v_i = va_arg(*pArgList, long);
    }
    else if (dataType == KRN_PRINTF_CTYPE_ULONG)
    {
        value.v_u = va_arg(*pArgList, unsigned long);
    }
    else if (dataType == KRN_PRINTF_CTYPE_LLONG)
    {
        value.v_i = va_arg(*pArgList, long long);
    }
    else if (dataType == KRN_PRINTF_CTYPE_ULLONG)
    {
        value.v_u = va_arg(*pArgList, unsigned long long);
    }
    else if (dataType == KRN_PRINTF_CTYPE_PTR)
    {
        value.v_ptr = va_arg(*pArgList, void*);
    }
    else if (dataType == KRN_PRINTF_CTYPE_SIZE)
    {
        value.v_u = va_arg(*pArgList, size_t);
    }
    else if (dataType == KRN_PRINTF_CTYPE_PTRDIFF)
    {
        value.v_i = va_arg(*pArgList, ptrdiff_t);
    }
    else if (dataType == KRN_PRINTF_CTYPE_I32)
    {
        value.v_i = va_arg(*pArgList, int32_t);
    }
    else if (dataType == KRN_PRINTF_CTYPE_U32)
    {
        value.v_u = va_arg(*pArgList, uint32_t);
    }
    else if (dataType == KRN_PRINTF_CTYPE_I64)
    {
        value.v_i = va_arg(*pArgList, int64_t);
    }
    else if (dataType == KRN_PRINTF_CTYPE_U64)
    {
        value.v_u = va_arg(*pArgList, uint64_t);
    }
    else
    {
        Krn_Err(KRN_ERR_INV_PRINTF_FORMAT, NULL);
        return 0;
    }
    
    // If this is a string value then handle that.
    if (typeId == KRN_PRINTF_TYPE_s)
    {
        // Output string in value.v_ptr
        const char* pString;
        int i;
        int bodyCount;
        int widthCount;
        int precisionCount;
        
        pString = (const char*) value.v_ptr;
        
        // Bind width and precision values to actual numbers.
        if (pSpec->Width == KRN_PRINTF_WIDTH_DEFAULT)
        {
            widthCount = 0;
        }
        else if (pSpec->Precision == KRN_PRINTF_WIDTH_ARG)
        {
            // Read it from an argument.
            widthCount = va_arg(*pArgList, int);
        }
        else
        {
            widthCount = pSpec->Width & KRN_PRINTF_WIDTH_MASK;
        }
        
        if (pSpec->Precision == KRN_PRINTF_WIDTH_DEFAULT)
        {
            precisionCount = INT_MAX;
        }
        else if (pSpec->Precision == KRN_PRINTF_WIDTH_ARG)
        {
            // Read it from an argument.
            precisionCount = va_arg(*pArgList, int);
        }
        else
        {
            precisionCount = pSpec->Precision & KRN_PRINTF_WIDTH_MASK;
        }
        
        // Figure out how large the string is.
        for (bodyCount = 0; (bodyCount < precisionCount) && (pString[bodyCount]); bodyCount++);
        
        // We are going to start writing data.  First check the alignment
        // flag.
        if (0 == (pSpec->Flags & KRN_PRINTF_FLAG_MINUS))
        {
            // Dump out some spaces first.
            for (i = bodyCount; i < widthCount; i++)
            {
                if (--MaxChars <= 0)
                {
                    break;
                }
                
                pOutFunc(pOutFuncContext, ' ');
                charsWritten++;
            }
        }
        
        // Now output the body.
        for (i = 0; i < bodyCount; i++)
        {
            if (--MaxChars <= 0)
            {
                break;
            }
            
            pOutFunc(pOutFuncContext, pString[i]);
            charsWritten++;
        }
        
        // Now padding if we are left-aligning.
        if (pSpec->Flags & KRN_PRINTF_FLAG_MINUS)
        {
            // Dump out some spaces first.
            for (i = bodyCount; i < widthCount; i++)
            {
                if (--MaxChars <= 0)
                {
                    break;
                }
                
                pOutFunc(pOutFuncContext, ' ');
                charsWritten++;
            }
        }
    }
    else if (typeId == KRN_PRINTF_TYPE_S)
    {
        // Output wide-string in value.v_ptr
        // Don't want ot support wide chars right now.
        return 0;
    }
    else if ((typeId == KRN_PRINTF_TYPE_d) ||
             (typeId == KRN_PRINTF_TYPE_i) ||
             (typeId == KRN_PRINTF_TYPE_o) ||
             (typeId == KRN_PRINTF_TYPE_u) ||
             (typeId == KRN_PRINTF_TYPE_x) ||
             (typeId == KRN_PRINTF_TYPE_X) ||
             (typeId == KRN_PRINTF_TYPE_p))
    {
        uint64_t tmpVal;
        const char* hexTable = &lowerHexTable[0];
        int i;
        int isNegative;
        int digitCount;
        int bodyCount;
        int widthCount;
        int precisionCount;
        
        // Now let's determine the number of digits we would need to display.
        // We use the umax_t type for the conversion.  First check if the 
        // value is negative.  Then convert it into an unsigned integer
        // (without hitting the nasty overflow case where there is one more 
        // negative number than there is positive in a signed integer).
        isNegative = 0;
        
        if (typeId & KRN_PRINTF_CTYPE_SIGNED)
        {
            // Check for negative.
            if (value.v_i < 0)
            {
                isNegative = 1;
                
                // It is negative, add 1 to make the abs a valid signed value.
                value.v_i = -(value.v_i + 1);
                
                // Now stuff this in the unsigned value.
                value.v_u = (uint64_t) value.v_i;
                
                // Last, add 1 to it so the abs is correct.
                value.v_u = value.v_u + 1;
            }
            else
            {
                // Easy, just stuff the value since it's positive.
                value.v_u = (uint64_t) value.v_i;
            }
        }
        
        // Bind width and precision values to actual numbers.
        if (pSpec->Width == KRN_PRINTF_WIDTH_DEFAULT)
        {
            widthCount = 0;
        }
        else if (pSpec->Width == KRN_PRINTF_WIDTH_ARG)
        {
            // Read it from an argument.
            widthCount = va_arg(*pArgList, int);
        }
        else
        {
            widthCount = pSpec->Width & KRN_PRINTF_WIDTH_MASK;
        }
        
        if (pSpec->Precision == KRN_PRINTF_WIDTH_DEFAULT)
        {
            if ((typeId == KRN_PRINTF_TYPE_s) ||
                (typeId == KRN_PRINTF_TYPE_S))
            {
                precisionCount = INT_MAX;
            }
            else if (typeId == KRN_PRINTF_TYPE_p)
            {
                precisionCount = sizeof(void*) * 2;
            }
            else
            {
                precisionCount = 1;
            }
        }
        else if (pSpec->Precision == KRN_PRINTF_WIDTH_ARG)
        {
            // Read it from an argument.
            precisionCount = va_arg(*pArgList, int);
            
            // Precision specified, clear the zero-padding width flag.
            pSpec->Flags &= ~(KRN_PRINTF_FLAG_ZERO);
        }
        else
        {
            precisionCount = pSpec->Precision & KRN_PRINTF_WIDTH_MASK;
            
            // Precision specified, clear the zero-padding width flag.
            pSpec->Flags &= ~(KRN_PRINTF_FLAG_ZERO);
        }
        
        // Setup the hex encoding table.  This also works for decimal.
        if (typeId == KRN_PRINTF_TYPE_X)
        {
            hexTable = &upperHexTable[0];
        }
        
        // At this point value.v_u has the abs of the value we will print.
        // Time to turn the number into digits stored in the TmpBuffer.
        digitCount = 0;
        bodyCount = 0;
        tmpVal = value.v_u;
        
        OS_ASSERT(divBase <= 16);
        
        while (tmpVal > 0)
        {
            uint64_t digitIndex;
            
            digitIndex = tmpVal % divBase;
            
            pSpec->TmpBuffer[bodyCount++] = hexTable[digitIndex];
            
            digitCount++;
            tmpVal /= divBase;
        }
        
        OS_ASSERT(bodyCount < 32);
        
        // Overload the precision value *IF* width was specified and there is
        // a zero flag.
        if ((pSpec->Width != KRN_PRINTF_WIDTH_DEFAULT) &&
            (pSpec->Flags & KRN_PRINTF_FLAG_ZERO))
        {
            precisionCount = widthCount;
            
            // Padding 0's takes care of the width requirement.
            widthCount = 0;
        }
        
        // Digits are laid out, but magnitude reversed.  This is useful, now
        // let's see if we didn't hit our precision goals.
        for (i = digitCount; i < precisionCount; i++)
        {
            pSpec->TmpBuffer[bodyCount++] = hexTable[0];
        }
        
        // Check if we are adding 0x, 0X, or 0 to it.
        if ((value.v_u > 0) &&
            (pSpec->Flags & KRN_PRINTF_FLAG_HASH))
        {
            if (typeId == KRN_PRINTF_TYPE_o)
            {
                pSpec->TmpBuffer[bodyCount++] = '0';
            }
            else if (typeId == KRN_PRINTF_TYPE_x)
            {
                // Remember: reversed.
                pSpec->TmpBuffer[bodyCount++] = 'x';
                pSpec->TmpBuffer[bodyCount++] = '0';
            }
            else if (typeId == KRN_PRINTF_TYPE_X)
            {
                // Remember: reversed.
                pSpec->TmpBuffer[bodyCount++] = 'X';
                pSpec->TmpBuffer[bodyCount++] = '0';
            }
        }
        
        // Check for +, only applicable to signed, positive values.
        if ((typeId == KRN_PRINTF_TYPE_i) ||
            (typeId == KRN_PRINTF_TYPE_d))
        {
            if (pSpec->Flags & KRN_PRINTF_FLAG_PLUS)
            {
                if (! isNegative)
                {
                    pSpec->TmpBuffer[bodyCount++] = '+';
                }
            }
            else if (pSpec->Flags & KRN_PRINTF_FLAG_SPACE)
            {
                if (! isNegative)
                {
                    pSpec->TmpBuffer[bodyCount++] = ' ';
                }
            }
        }
        
        // Check for negative?
        if (isNegative)
        {
            pSpec->TmpBuffer[bodyCount++] = '-';
        }
        
        // Okay, now the whole thing should be in tmpBuffer, but reversed.
        
        // We are going to start writing data.  First check the alignment
        // flag.
        if (0 == (pSpec->Flags & KRN_PRINTF_FLAG_MINUS))
        {
            // Dump out some spaces first.
            for (i = bodyCount; i < widthCount; i++)
            {
                if (--MaxChars <= 0)
                {
                    break;
                }
                
                pOutFunc(pOutFuncContext, ' ');
                charsWritten++;
            }
        }
        
        // Now output the body.
        for (i = bodyCount-1; i >= 0; i--)
        {
            if (--MaxChars <= 0)
            {
                break;
            }
            
            pOutFunc(pOutFuncContext, pSpec->TmpBuffer[i]);
            charsWritten++;
        }
        
        // Now padding if we are left-aligning.
        if (pSpec->Flags & KRN_PRINTF_FLAG_MINUS)
        {
            // Dump out some spaces first.
            for (i = bodyCount; i < widthCount; i++)
            {
                if (--MaxChars <= 0)
                {
                    break;
                }
                
                pOutFunc(pOutFuncContext, ' ');
                charsWritten++;
            }
        }
    }
    
    return charsWritten;
}

void
OSCALL
Krn_PrintfDefaultStringWriter(
    void* pContext,
    char Character
    )
{
    KRN_PRINTF_DEF_STRING_WRITER_CONTEXT* pStringContext;
    
    pStringContext = (KRN_PRINTF_DEF_STRING_WRITER_CONTEXT*) pContext;
    
    pStringContext->pBuffer[pStringContext->Offset++] = Character;
}

// 
// External functions.
// 
int
OSCALL
Krn_vsnprintf(
    char* pDest,
    size_t DestSize,
    const char* pFormat,
    va_list args
    )
{
    KRN_PRINTF_DEF_STRING_WRITER_CONTEXT context = {0};
    
    context.pBuffer = pDest;
    
    return Krn_vsnprintf_func(
            Krn_PrintfDefaultStringWriter,
            &context,
            DestSize,
            pFormat,
            args);
}

int
OSCALL
Krn_vsnprintf_func(
    KRN_PRINTF_CHAR_WRITER_FUNC pOutFunc,
    void* pOutFuncContext,
    size_t MaxSize,
    const char* pFormat,
    va_list args
    )
{
    KRN_PRINTF_VAL_SPEC currentSpec = {0};
    size_t i;
    size_t charsWritten = 0;
    
    i = 0;
    
    while (pFormat[i])
    {
        if (pFormat[i] == '%')
        {
            i++;
            
            if (pFormat[i] == '%')
            {
                if (--MaxSize <= 0)
                {
                    break;
                }
                
                pOutFunc(pOutFuncContext, '%');
                charsWritten++;
                
                i++;
            }
            else
            {
                size_t tmpUsed;
                
                tmpUsed = Krn_PrintfDecodeSpec(&pFormat[i], &currentSpec);
                if (tmpUsed <= 0)
                {
                    break;
                }
                
                i += tmpUsed;
                
                // Check for special %n.
                if (KRN_PRINTF_TYPE_n == (currentSpec.Flags & KRN_PRINTF_TYPE_MASK))
                {
                    int* pCount;
                    
                    pCount = (int*) va_arg(args, int*);
                    *pCount = (int) charsWritten;
                }
                else
                {
                    // Now print it.
                    tmpUsed = Krn_PrintfOutputSpec(
                            pOutFunc,
                            pOutFuncContext,
                            MaxSize,
                            &currentSpec,
                            &args);
                    
                    if (tmpUsed > MaxSize)
                    {
                        break;
                    }
                    MaxSize -= tmpUsed;
                }
            }
        }
        else
        {
            if (--MaxSize <= 0)
            {
                break;
            }
            
            pOutFunc(pOutFuncContext, pFormat[i]);
            charsWritten++;
            
            i++;
        }
    }
    
    return i;
}



