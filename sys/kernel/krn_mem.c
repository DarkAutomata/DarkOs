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
#include <krn_mem.h>

typedef struct _MEM_PAGE_DB_ENTRY MEM_PAGE_DB_ENTRY;
typedef struct _MEM_BUDDY_LIST MEM_BUDDY_LIST;

#define MEM_PAGE_0_FLAG_USED    0x80000000
#define MEM_PAGE_0_FLAG_CRIT    0x40000000      // System critical
#define MEM_PAGE_0_MASK_TYPE    0x0F000000
#define MEM_PAGE_0_MASK_KRN     0x00FFFFFF
#define MEM_PAGE_1_MASK_USR     0x00FFFFFF

struct _MEM_PAGE_DB_ENTRY
{
    uint32_t PageFlags0;
    uint32_t PageFlags1;
};

#define MEM_BUDDY_MAX_BLOCKS            (256*1024*1024)
#define MEM_BUDDY_BITS_PER_ELEMENT      (8 * sizeof(uint32_t))

struct _MEM_BUDDY_LIST
{
    uint32_t BlockCount;
    uint32_t BlockSize;
    uint32_t BitsFree;
    uint32_t* pBlockBits;
    MEM_BUDDY_LIST* pPrev;
    MEM_BUDDY_LIST* pNext;
};


MEM_BUDDY_LIST*
OSCALL
Mem_InitBuddyList(
    void* pLocation,
    size_t LocationSize,
    uint32_t BlockCount,
    size_t* pBytesUsed
    );

KRN_ERROR_CODE
OSCALL
Mem_BuddyAllocBlocks(
    MEM_BUDDY_LIST* pList,
    uint32_t BlockCount,
    uint32_t* pBlockStart
    );

KRN_ERROR_CODE
OSCALL
Mem_BuddyFreeBlocks(
    MEM_BUDDY_LIST* pList,
    uint32_t pBlockStart,
    uint32_t BlockCount
    );

void
OSCALL
Mem_BuddySetBit(
    MEM_BUDDY_LIST* pList,
    uint32_t BlockIndex
    );

void
OSCALL
Mem_BuddyClearBit(
    MEM_BUDDY_LIST* pList,
    uint32_t BlockIndex
    );

// Implementation

MEM_BUDDY_LIST*
OSCALL
Mem_InitBuddyList(
    void* pLocation,
    size_t LocationSize,
    uint32_t BlockCount,
    size_t* pBytesUsed
    )
{
    MEM_BUDDY_LIST* pTopList;
    char* pLocationBytes;
    uint32_t blockSize;
    size_t bytesUsed;
    uint32_t i;
    uint32_t listCount;
    
    pTopList = NULL;
    bytesUsed = 0;
    *pBytesUsed = 0;
    pLocationBytes = (char*) pLocation;
    listCount = 0;
    
    // Figure out the total number of bits required to store the lists.
    // First, round up the bit count to a power of 2, counting along the
    // way.
    
    blockSize = 1;
    // Round to power of 2.
    while ((blockSize < MEM_BUDDY_MAX_BLOCKS) &&
           (blockSize < BlockCount))
    {
        blockSize *= 2;
    }
    
    // Too large for us.
    if (blockSize >= (MEM_BUDDY_MAX_BLOCKS))
    {
        return NULL;
    }
    
    // Figure out the number of block levels we will use.  The first level
    // should have a pretty decent number of blocks in it -- enough to utilize
    // a full bit block element.
    blockSize /= MEM_BUDDY_BITS_PER_ELEMENT;
    
    if (blockSize == 0)
    {
        // This is designed to hold more than just a few elements, bail out.
        return NULL;
    }
    
    // Fill out the headers up front.
    pTopList = (MEM_BUDDY_LIST*) &pLocationBytes[bytesUsed];
    i = 0;
    
    while (blockSize > 0)
    {
        bytesUsed += sizeof(MEM_BUDDY_LIST);
        if (bytesUsed > LocationSize)
        {
            return NULL;
        }
        
        pTopList[i].BlockCount = BlockCount / blockSize;
        pTopList[i].BlockSize = blockSize;
        pTopList[i].BitsFree = 0;
        pTopList[i].pBlockBits = NULL;
        
        if (i == 0)
        {
            pTopList[i].pPrev = NULL;
        }
        else
        {
            pTopList[i].pPrev = &pTopList[i-1];
            pTopList[i].pPrev->pNext = &pTopList[i];
        }
        
        pTopList[i].pNext = NULL;
        
        blockSize /= 2;
        i++;
    }
    listCount = i;
    
    // Now fill out the block bit pointers and zero everything.
    for (i = 0; i < listCount; i++)
    {
        uint32_t elementCount;
        uint32_t j;
        
        pTopList[i].pBlockBits = (uint32_t*) &pLocationBytes[bytesUsed];
        
        // Initialize the block bits to busy.  We are adding 1 here to ensure
        // any remainders for the blockCount division are accounted for.
        // NOTE: There shouldn't be any remainders since this will currently
        // be a power of two, but this is just to be safe if that changes
        // later.
        elementCount = 1 + (pTopList[i].BlockCount / sizeof(uint32_t));
        
        bytesUsed += sizeof(uint32_t) * elementCount;
        
        if (bytesUsed > LocationSize)
        {
            return NULL;
        }
        
        for (j = 0; j < elementCount; j++)
        {
            pTopList[i].pBlockBits[j] = 0xFFFFFFFF;
        }
    }
    
    // Last, mark the whole provided bit range as free.
    Mem_BuddyFreeBlocks(pTopList, 0, BlockCount);
    
    *pBytesUsed = bytesUsed;
    
    return pTopList;
}


KRN_ERROR_CODE
OSCALL
Mem_BuddyAllocBlocks(
    MEM_BUDDY_LIST* pList,
    uint32_t BlockCount,
    uint32_t* pBlockStart
    )
{
    uint32_t i;
    uint32_t blockStart;
    
    MEM_BUDDY_LIST* pCurrent;
    
    *pBlockStart = 0;
    
    // Find the right level to do the allocation.
    pCurrent = pList;
    while ((pCurrent->pNext) &&
           (pCurrent->BlockSize > BlockCount))
    {
        pCurrent = pCurrent->pNext;
    }
    
    // Make sure there are free blocks in this level, if not then we need to
    // split a larger block.
    while ((pCurrent) &&
           (pCurrent->BitsFree == 0))
    {
        pCurrent = pCurrent->pPrev;
    }
    
    if (! pCurrent)
    {
        // We can't satisfy this request right now.
        return KRN_ERR_NOT_ENOUGH_MEM;
    }
    
    // There are free bits at this level, find a block.
    for (i = 0; i < pCurrent->BlockCount; i++)
    {
        uint32_t elementIndex = i / MEM_BUDDY_BITS_PER_ELEMENT;
        uint32_t bitIndex = i % MEM_BUDDY_BITS_PER_ELEMENT;
        
        if ((pCurrent->pBlockBits[elementIndex] & (1UL << bitIndex)) == 0)
        {
            break;
        }
    }
    
    // We should have found one... for sure.
    OS_ASSERT(i < pCurrent->BlockCount);
    
    blockStart = i;
    
    // Go down to the lowest level now, multiply blockStart by 2 each time.
    while (pCurrent->pNext)
    {
        pCurrent = pCurrent->pNext;
        blockStart *= 2;
    }
    
    // Mark the lowest level set, then propogate up as needed.
    for (i = blockStart; i < (blockStart + BlockCount); i++)
    {
        Mem_BuddySetBit(pCurrent, i);
    }
    
    *pBlockStart = blockStart;
    
    return KRN_ERR_SUCCESS;
}

KRN_ERROR_CODE
OSCALL
Mem_BuddyFreeBlocks(
    MEM_BUDDY_LIST* pList,
    uint32_t BlockStart,
    uint32_t BlockCount
    )
{
    uint32_t i;
    MEM_BUDDY_LIST* pCurrent;
    
    // Find the end (lowest level).
    pCurrent = pList;
    while (pCurrent->pNext)
    {
        pCurrent = pCurrent->pNext;
    }
    
    // Check bounds.
    if ((BlockStart >= pCurrent->BlockCount) ||
        (BlockCount >= pCurrent->BlockCount) ||
        (BlockCount < BlockStart) ||
        ((BlockCount - BlockStart) > pCurrent->BlockCount))
    {
        return KRN_ERR_INV_PARAMETER;
    }
    
    // Mark the lowest level free, then propogate up as needed.
    for (i = BlockStart; i < (BlockStart + BlockCount); i++)
    {
        Mem_BuddyClearBit(pCurrent, i);
    }
    
    return KRN_ERR_SUCCESS;
}

void
OSCALL
Mem_BuddySetBit(
    MEM_BUDDY_LIST* pList,
    uint32_t BlockIndex
    )
{
    while (pList)
    {
        uint32_t elementIndex = BlockIndex / MEM_BUDDY_BITS_PER_ELEMENT;
        uint32_t bitIndex = BlockIndex % MEM_BUDDY_BITS_PER_ELEMENT;
        
        // Set the bit.
        pList->pBlockBits[elementIndex] |= 1UL << bitIndex;
        pList->BitsFree--;
        
        bitIndex &= ~0x01;  // Zero least significant bit.
        
        // Are both these bits set now?
        if ((pList->pBlockBits[elementIndex] & (3UL << bitIndex)) == (3UL << bitIndex))
        {
            // Jump up a level, and divide BlockIndex for the correct index in the
            // next level.
            pList = pList->pPrev;
            BlockIndex /= 2;
        }
        else
        {
            break;
        }
    }
}

void
OSCALL
Mem_BuddyClearBit(
    MEM_BUDDY_LIST* pList,
    uint32_t BlockIndex
    )
{
    while (pList)
    {
        uint32_t elementIndex = BlockIndex / MEM_BUDDY_BITS_PER_ELEMENT;
        uint32_t bitIndex = BlockIndex % MEM_BUDDY_BITS_PER_ELEMENT;
        
        // Clear the bit.
        pList->pBlockBits[elementIndex] &= ~(1UL << bitIndex);
        pList->BitsFree++;
        
        bitIndex &= ~0x01;  // Zero least significant bit.
        
        // Are both these bits clear now?
        if ((pList->pBlockBits[elementIndex] & (3UL << bitIndex)) == 0)
        {
            // Jump up a level, and divide BlockIndex for the correct index in the
            // next level.
            pList = pList->pPrev;
            BlockIndex /= 2;
        }
        else
        {
            break;
        }
    }
}

