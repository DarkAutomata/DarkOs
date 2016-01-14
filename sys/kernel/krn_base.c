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

// Kernel mode base functionality.

#include <krn_base.h>

void*
OSCALL
memset(
    void* pDest,
    int Value,
    size_t Count
    )
{
    char* pDestChar = (char*) pDest;
    
    while (Count-- > 0)
    {
        *pDestChar++ = (char) Value;
    }
    
    return pDest;
}

uint32_t g_KrnCheckPoint = 1;

void
OSCALL
Krn_CheckPoint(
    )
{
    while (g_KrnCheckPoint)
    {
    }
}

// Interlocked functions.
uint32_t
OSCALL
Krn_InterlockedInc32(
    uint32_t* pDest
    )
{
    return __sync_fetch_and_add(pDest, 1);
}

uint64_t
OSCALL
Krn_InterlockedInc64(
    uint32_t* pDest
    )
{
    return __sync_fetch_and_add(pDest, 1ULL);
}

uint32_t
OSCALL
Krn_InterlockedDec32(
    uint32_t* pDest
    )
{
    return __sync_fetch_and_add(pDest, -1);
}

uint64_t
OSCALL
Krn_InterlockedDec64(
    uint64_t* pDest
    )
{
    return __sync_fetch_and_add(pDest, -1LL);
}

uint32_t
OSCALL
Krn_InterlockedAdd32(
    uint32_t* pDest,
    uint32_t Value
    )
{
    return __sync_fetch_and_add(pDest, Value);
}

uint64_t
OSCALL
Krn_InterlockedAdd64(
    uint32_t* pDest,
    uint32_t Value
    )
{
    return __sync_fetch_and_add(pDest, Value);
}

uint32_t
OSCALL
Krn_InterlockedExg32(
    uint32_t* pDest,
    uint32_t Value
    )
{
    return __sync_lock_test_and_set(pDest, Value);
}

uint64_t
OSCALL
Krn_InterlockedExg64(
    uint64_t* pDest,
    uint64_t Value
    )
{
    return __sync_lock_test_and_set(pDest, Value);
}

uint32_t
OSCALL
Krn_InterlockedCmpExg32(
    uint32_t* pDest,
    uint32_t Value,
    uint32_t Compare
    )
{
    return __sync_val_compare_and_swap(pDest, Compare, Value);
}

uint64_t
OSCALL
Krn_InterlockedCmpExg64(
    uint64_t* pDest,
    uint64_t Value,
    uint64_t Compare
    )
{
    return __sync_val_compare_and_swap(pDest, Compare, Value);
}

void*
OSCALL
Krn_InterlockedExgPtr(
    void** pDest,
    void* Value
    )
{
    return __sync_lock_test_and_set(pDest, Value);
}

void*
OSCALL
Krn_InterlockedCmpExgPtr(
    void** pDest,
    void* Value,
    void* Compare
    )
{
    return __sync_val_compare_and_swap(pDest, Compare, Value);
}

// Stack functions.
void
OSCALL
Krn_StackInit(
    STACK_ENTRY* pStack
    )
{
    pStack->pNext = NULL;
}

void
OSCALL
Krn_StackPush(
    STACK_ENTRY* pStack,
    STACK_ENTRY* pEntry
    )
{
    STACK_ENTRY* pOldVal;
    STACK_ENTRY* pExcVal;
    
    // Atomic operation.
    do
    {
        pOldVal = pStack->pNext;
        pEntry->pNext = pOldVal;
        
        pExcVal = Krn_InterlockedCmpExgPtr(
                (void**) &pStack->pNext,
                pEntry,
                pOldVal);
    } while (pExcVal != pOldVal);
}

STACK_ENTRY*
OSCALL
Krn_StackPop(
    STACK_ENTRY* pStack
    )
{
    STACK_ENTRY* pOldVal;
    STACK_ENTRY* pExcVal;
    
    // Atomic operation.
    do
    {
        pOldVal = pStack->pNext;
        
        if (pOldVal)
        {
            pExcVal = Krn_InterlockedCmpExgPtr(
                    (void**) &pStack->pNext,
                    pOldVal->pNext,
                    pOldVal);
        }
        else
        {
            // Empty.
            pOldVal = NULL;
            break;
        }
    } while (pExcVal != pOldVal);
    
    return pOldVal;
}

// List functions.
void
OSCALL
Krn_ListInit(
    LIST_ENTRY* pList
    )
{
    // Initialize to itself.
    pList->pNext = pList;
    pList->pPrev = pList;
}

void
OSCALL
Krn_ListAddHead(
    LIST_ENTRY* pList,
    LIST_ENTRY* pEntry
    )
{
    pEntry->pNext = pList->pNext;
    pEntry->pPrev = pList;
    
    // Update pEntry neighbors.
    pEntry->pNext->pPrev = pEntry;
    pEntry->pPrev->pNext = pEntry;
}

void
OSCALL
Krn_ListAddTail(
    LIST_ENTRY* pList,
    LIST_ENTRY* pEntry
    )
{
    pEntry->pNext = pList;
    pEntry->pPrev = pList->pPrev;
    
    // Update pEntry neighbors.
    pEntry->pNext->pPrev = pEntry;
    pEntry->pPrev->pNext = pEntry;
}

LIST_ENTRY*
OSCALL
Krn_ListRemoveHead(
    LIST_ENTRY* pList
    )
{
    LIST_ENTRY* pEntry;
    
    if (pList->pNext == pList)
    {
        return NULL;
    }
    
    pEntry = pList->pNext;
    
    pList->pNext = pList->pNext->pNext;
    pList->pNext->pPrev = pList;
    
    return pEntry;
}

LIST_ENTRY*
OSCALL
Krn_ListRemoveTail(
    LIST_ENTRY* pList
    )
{
    LIST_ENTRY* pEntry;
    
    if (pList->pPrev == pList)
    {
        return NULL;
    }
    
    pEntry = pList->pPrev;
    
    pList->pPrev = pList->pPrev->pPrev;
    pList->pPrev->pNext = pList;
    
    return pEntry;
}

void
OSCALL
Krn_ListRemoveEntry(
    LIST_ENTRY* pEntry
    )
{
    pEntry->pNext->pPrev = pEntry->pPrev;
    pEntry->pPrev->pNext = pEntry->pNext;
}

void
OSCALL
Krn_ListAddEntry(
    LIST_ENTRY* pBefore,
    LIST_ENTRY* pEntry
    )
{
    pEntry->pNext = pBefore;
    pEntry->pPrev = pBefore->pPrev;
    
    pEntry->pNext->pPrev = pEntry;
    pEntry->pPrev->pNext = pEntry;
}

