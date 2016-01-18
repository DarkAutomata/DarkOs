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
typedef uint32_t KRN_ERROR_CODE;

#define KRN_ERR_FATAL_MASK                  0x80000000

#define KRN_ERR_SUCCESS                     (0x00000000)

#define KRN_ERR_ASSERT_FAILED               (0x10000001 | KRN_ERR_FATAL_MASK)
#define KRN_ERR_INV_PRINTF_FORMAT           (0x10000002 | KRN_ERR_FATAL_MASK)
#define KRN_ERR_NOT_ENOUGH_MEM              (0x10000003)
#define KRN_ERR_INV_PARAMETER               (0x10000004)

// 
// Various kernel data structures.
//

typedef struct _KRN_STACK_ENTRY KRN_STACK_ENTRY;
typedef struct _KRN_LIST_ENTRY KRN_LIST_ENTRY;

struct _KRN_STACK_ENTRY
{
    KRN_STACK_ENTRY* pNext;
};

//
// In this doubly-linked list structure, the "ListHead" is itself a ListEntry.
// However, it is a sentinel which points to itself at init.  This creates a
// circular list structure, where the "head" signifies the first/last insert
// location.  If Head->pNext == Head or Head->pPrev == Head then the end of
// iteration has occurred.
// 
struct _KRN_LIST_ENTRY
{
    KRN_LIST_ENTRY* pNext;
    KRN_LIST_ENTRY* pPrev;
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
    KRN_STACK_ENTRY* pStack
    );

void
OSCALL
Krn_StackPush(
    KRN_STACK_ENTRY* pStack,
    KRN_STACK_ENTRY* pEntry
    );

KRN_STACK_ENTRY*
OSCALL
Krn_StackPop(
    KRN_STACK_ENTRY* pStack
    );

// List functions.
void
OSCALL
Krn_ListInit(
    KRN_LIST_ENTRY* pList
    );

void
OSCALL
Krn_ListAddHead(
    KRN_LIST_ENTRY* pList,
    KRN_LIST_ENTRY* pEntry
    );

void
OSCALL
Krn_ListAddTail(
    KRN_LIST_ENTRY* pList,
    KRN_LIST_ENTRY* pEntry
    );

KRN_LIST_ENTRY*
OSCALL
Krn_ListRemoveHead(
    KRN_LIST_ENTRY* pList
    );

KRN_LIST_ENTRY*
OSCALL
Krn_ListRemoveTail(
    KRN_LIST_ENTRY* pList
    );

void
OSCALL
Krn_ListRemoveEntry(
    KRN_LIST_ENTRY* pEntry
    );

void
OSCALL
Krn_ListAddEntry(
    KRN_LIST_ENTRY* pBefore,
    KRN_LIST_ENTRY* pEntry
    );

#endif // __KRN_BASE_H__


