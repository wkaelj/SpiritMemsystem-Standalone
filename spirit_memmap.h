#pragma once

#include <stdio.h>
#include "spirit_memory.h"

// Functions to aid in the managment of memory maps
// Store a map of a 4096 byte block of memory by mapping it byte for byte
// into a array, with each byte being represented as a single bit.
//
// Individual variables are seperated by a single bit, so that they can be automatically freed.
//
//
// Kael Johnston March 14 2022

// debug
#ifdef MEMORY_DEBUG
//#define ENABLE_PRINT_BINARY
#endif

#define BLOCK_SIZE 64

// total number of bytes in a block (8-bit)
#define BLOCK_BYTES_TOTAL 4096

// store and index blocks of memory
typedef struct t_memblock {
    uint32_t blockid;
    b64 mapbits[BLOCK_SIZE]; // essentialy a 2D array 64x64 bits
    b64 mapmem[BLOCK_BYTES_TOTAL/8];

    struct t_memblock *pNext;
} memblock;

// set a chunk of memory in a memblock
// Params:
//      void *index - a pointer to the memory to set
//      size_t size - the amount of memory to set (8-bit bytes)
//      uint_fast8_t TorF - if the memory will be set 1s or 0s. any !0 value is 1
// Returns:
//      0 for success, !0 for failure
uint_fast8_t setPtr (memblock *hd, void *index, size_t size, uint_fast8_t bitVal);

// find a gap of at least size, and return its index in the array
uint64_t findPtr (memblock *hd, const size_t minSize);

// check if a given bit is set
// Params:
//      void *index - the index to check
// Returns:
//      0 for no, or the size of the chunk (8-bit bytes) - 16-bit return bc 255 cannot index. I suspect 64-bit anyway.
uint_fast16_t checkBit (memblock *hd, void *index);

// check if a bit is true or false, without returning the size
uint_fast8_t checkBitFast (memblock *hd, void *index);

// remove any empty memblocks from the tree, and return number of blocks removed
uint_fast8_t deBloat (memblock *hd);

// print all of the memory blocks to stdout
uint_fast8_t dumpMap (memblock *hd);
