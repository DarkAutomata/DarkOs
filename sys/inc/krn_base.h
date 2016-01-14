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

#ifndef __KRN_BASE_H__
#define __KRN_BASE_H__

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

// 
// Helpful utility macros.
//
#define _countof(X)     (sizeof(X) / sizeof((X)[0]))

// Place-holder for now.
#define OSCALL

// 
// Kernel error codes and reporting.
// 
#define KRN_ERR_FATAL_MASK                  0x80000000

#define KRN_ERR_ASSERT_FAILED               (0x00000001 | KRN_ERR_FATAL_MASK)
#define KRN_ERR_INV_PRINTF_FORMAT           (0x00000002 | KRN_ERR_FATAL_MASK)

// 
// Various kernel data structures.
//

typedef struct _STACK_ENTRY STACK_ENTRY;
typedef struct _LIST_ENTRY LIST_ENTRY;

struct _STACK_ENTRY
{
    STACK_ENTRY* pNext;
};

//
// In this doubly-linked list structure, the "ListHead" is itself a ListEntry.
// However, it is a sentinel which points to itself at init.  This creates a
// circular list structure, where the "head" signifies the first/last insert
// location.  If Head->pNext == Head or Head->pPrev == Head then the end of
// iteration has occurred.
// 
struct _LIST_ENTRY
{
    LIST_ENTRY* pNext;
    LIST_ENTRY* pPrev;
};

// 
// Functions
// 
void
OSCALL
Krn_ErrFunc(
    uint32_t ErrorCode,
    const char* pSourceFile,
    int SourceLine,
    const void* pParam
    );

#define Krn_Err(ErrorCode, Param) \
    Krn_ErrFunc(ErrorCode, __FILE__, __LINE__, Param)

#define OS_ASSERT(X)        if (!(X)) { Krn_Err(KRN_ERR_ASSERT_FAILED, NULL); }

void*
OSCALL
memset(
    void* pDest,
    int Value,
    size_t Count
    );

void
OSCALL
Krn_CheckPoint(
    );

// Interlocked functions.
uint32_t
OSCALL
Krn_InterlockedInc32(
    uint32_t* pDest
    );

uint64_t
OSCALL
Krn_InterlockedInc64(
    uint32_t* pDest
    );

uint32_t
OSCALL
Krn_InterlockedDec32(
    uint32_t* pDest
    );

uint64_t
OSCALL
Krn_InterlockedDec64(
    uint64_t* pDest
    );

uint32_t
OSCALL
Krn_InterlockedAdd32(
    uint32_t* pDest,
    uint32_t Value
    );

uint64_t
OSCALL
Krn_InterlockedAdd64(
    uint32_t* pDest,
    uint32_t Value
    );

uint32_t
OSCALL
Krn_InterlockedExg32(
    uint32_t* pDest,
    uint32_t Value
    );

uint64_t
OSCALL
Krn_InterlockedExg64(
    uint64_t* pDest,
    uint64_t Value
    );

uint32_t
OSCALL
Krn_InterlockedCmpExg32(
    uint32_t* pDest,
    uint32_t Value,
    uint32_t Compare
    );

uint64_t
OSCALL
Krn_InterlockedCmpExg64(
    uint64_t* pDest,
    uint64_t Value,
    uint64_t Compare
    );

void*
OSCALL
Krn_InterlockedExgPtr(
    void** pDest,
    void* Value
    );

void*
OSCALL
Krn_InterlockedCmpExgPtr(
    void** pDest,
    void* Value,
    void* Compare
    );

// Stack functions.
void
OSCALL
Krn_StackInit(
    STACK_ENTRY* pStack
    );

void
OSCALL
Krn_StackPush(
    STACK_ENTRY* pStack,
    STACK_ENTRY* pEntry
    );

STACK_ENTRY*
OSCALL
Krn_StackPop(
    STACK_ENTRY* pStack
    );

// List functions.
void
OSCALL
Krn_ListInit(
    LIST_ENTRY* pList
    );

void
OSCALL
Krn_ListAddHead(
    LIST_ENTRY* pList,
    LIST_ENTRY* pEntry
    );

void
OSCALL
Krn_ListAddTail(
    LIST_ENTRY* pList,
    LIST_ENTRY* pEntry
    );

LIST_ENTRY*
OSCALL
Krn_ListRemoveHead(
    LIST_ENTRY* pList
    );

LIST_ENTRY*
OSCALL
Krn_ListRemoveTail(
    LIST_ENTRY* pList
    );

void
OSCALL
Krn_ListRemoveEntry(
    LIST_ENTRY* pEntry
    );

void
OSCALL
Krn_ListAddEntry(
    LIST_ENTRY* pBefore,
    LIST_ENTRY* pEntry
    );

#endif // __KRN_BASE_H__


