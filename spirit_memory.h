#pragma once
#include <stdlib.h>
#include <stdint.h>

// Memory subsytem
// Seperates all blocks of memory by 1 byte, to ensure does not free adjacent blocks.
// more efficient then storeing jumps as 32-bit uints
//
//
// Kael Johnston March 14 2022

// debug
#define MEMORY_DEBUG

#define b64 uint64_t
#define b8 unsigned char
#define B64_MAX 0xFFFFFFFFFFFFFFFF
#define B8_MAX 0xff

// function definitions

// must be used to dereference a pointer
#define dref(ptr) (*(typeof(ptr))spMemDeref(ptr))

// don't like typing
#define alloc(size) spMemAlloc(size, SPIRIT_MEM_PERM)
#define alloc_tmp(size) spMemAlloc(size, SPIRIT_MEM_TMP)

typedef enum e_SpiritMemtype {
    SPIRIT_MEM_TMP,
    SPIRIT_MEM_PERM,

    SPIRIT_MEMTYPES_MAX
} SpiritMemtype;

// allocate memory with size min
void *spMemAlloc (size_t min, SpiritMemtype lifespan);

// set a block of memory to zero
void *spMemZero (void *mem);

// must be used to dereference pointer, because the *
// operator will attempt to reference system memory, which is almost
// garunteed to be unavailable
void *spMemDeref (void *mem);

// get the size of a given block of memory
size_t spMemGetSize (void *mem);

// free a block of memory
int spFree (void *mem);

