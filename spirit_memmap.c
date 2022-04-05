#include "spirit_memmap.h"

#include <stdio.h>
// Memory mapping implementation
//
//
// Kael Johnston March 16 2022


// very hacked debug stuff
#ifdef ENABLE_PRINT_BINARY
#define db_print_binary(value, size) printBinary(value, size)
#else
#define db_print_binary(value, size)
#endif

#ifdef ENABLE_PRINT_BINARY
static void printBinary (b64 val, size_t bitcount) {
    char out[bitcount + 1];
    out[bitcount] = '\0';
    for (size_t i = 1; i < bitcount; i++) {
        if (val & 1) out[bitcount - (i + 1)] = '1';
        else out[bitcount - (i + 1)] = '0';
        val = val >> 1;
    }
    printf ("%s\n", out);
}
#endif

uint_fast8_t setPtr (memblock *hd, void *index, size_t size, uint_fast8_t TorF ) {

    b64 ptr = (uint64_t) index;

    // convert pointer to per-block index and traverse to correct block
    while (ptr >= BLOCK_BYTES_TOTAL) {
        if (hd->pNext == NULL) return 1; // cannot create new block, memory won't be consecutive
        hd = hd->pNext;
        ptr -= BLOCK_BYTES_TOTAL;
    }

    // pointer conversion operations
    b64 *rowptr = &hd->mapbits[ptr >> 6]; // row = ptr / 64
    ptr &= 0x3f; // pointer is now row specific (ptr % 64)
    if (ptr + size >= BLOCK_SIZE) {
        uint64_t blockOffset = BLOCK_SIZE - ptr;
        if (setPtr(hd, index + blockOffset, size - blockOffset, TorF) != 0) return 1; // will need to write to next row
    }

    // create bitmask
    b64 bitmask;
    // if the number will be bit shifted by < 63, set it to 0 because shifting by > 63 is undefined
    if (size < 63) bitmask = (B64_MAX << ptr) << size;
    else bitmask = 0x0;

    bitmask |= ~(B64_MAX << ptr); // get rid of the end of the line (0b1101)
    if (TorF) bitmask = ~bitmask; // invert bitmask to set true bits (0b0010)
    // set row
    if (TorF) *rowptr |= bitmask; // setting 1s
    else *rowptr &= bitmask; // run if setting 0s

    return 0;
}

// find a pointer
uint64_t findPtr (memblock *hd, const size_t minSize) {

    if (minSize >= 4096) {
        // get make better memory?
        return UINT64_MAX;
    }

    // variables
    uint_fast8_t bufferInt; // used for random stuff that doesn't deserve a variable
    uint_fast8_t found = 1; // 0-1 wether memory has been found
    uint_fast8_t currentRow = 0; // current row of memory being parsed
    b64 testingByte = hd->mapbits[0]; // manipulate without breaking memory map
    uint_fast8_t currentIndex = 0; // store the size of the current chunk
    // TODO handle size over lines
    while (found) {
        // handle ctz undefined behavoir for 0
        if (__builtin_ctz (testingByte) > 0) {
            // handle single bit gaps between variables
            if (testingByte & 0x2){
                testingByte >>= 1; // handle the single-bit gaps between variable
                currentIndex++;
            } else
            // handle gaps that are big enough for the variable
            // the | ~(B64_MAX >> currentIndex) is because if the map has been shifted left,
            // the trailing 0s might be treated as empty space
            if (__builtin_clz (testingByte | ~(B64_MAX >> currentIndex)) - 2 >= minSize) {
                return currentIndex + 1 + currentRow * 64;
            }
            // handle gaps that do not fit the variable
            else {
                bufferInt = __builtin_clz (testingByte | ~(B64_MAX) >> currentIndex);
                bufferInt += currentIndex;
                if (bufferInt >= 64) {
                    currentRow++;
                    currentIndex = currentIndex;
                    testingByte = hd->mapbits[currentRow];
                } else currentIndex = bufferInt;
            }
        }
        // move to next gap
        else {
            bufferInt = __builtin_ctz(~testingByte);
            testingByte >>= bufferInt; // move to next gap
            bufferInt += currentIndex;
            if (bufferInt >= 64) {
                currentRow++; //next line
                currentIndex = 0;
                printf ("new row: %u\n", currentRow);
                testingByte = hd->mapbits[currentRow];
            } else currentIndex = bufferInt;
        }
    }
    printf ("allocation failure");
    return UINT64_MAX;
}

uint_fast8_t checkBitFast (memblock *hd, void *index) {

    uint64_t ptr = (uint64_t) index;
    while (ptr >= BLOCK_BYTES_TOTAL) {
        if (hd->pNext == NULL) return 0;
        hd = hd->pNext;
        ptr -= BLOCK_BYTES_TOTAL;
    }
    b64 *rowptr = &hd->mapbits[ptr >> 6]; // ptr / 64
    ptr &= 0x3f; // %64

    // check there is a bit set at the specified index of the map
    b64 parsingBit = 0x1 << ptr;
    if (parsingBit & *rowptr) {
        return 1;
    }
    return 0;
}

// check a memory location and return the size
uint_fast16_t checkBit (memblock *hd, void *index) {

    uint64_t ptr = (uint64_t) index;
    while (ptr >= BLOCK_BYTES_TOTAL) {
        if (hd->pNext == NULL) return 0;
        hd = hd->pNext;
        ptr -= BLOCK_BYTES_TOTAL;
    }
    b64 *rowptr = &hd->mapbits[ptr >> 6]; // ptr / 64
    ptr &= 0x3f; // %64

    // check there is a bit set at the specified index of the map
    b64 parsingBit = 0x1 << ptr;
    if (parsingBit & *rowptr) {
        uint_fast16_t size = 1;
        while ((parsingBit <<= 1) & *rowptr) size++; // iterate until 0
        return size;
    }
    // else return 0
    return 0;
}


// Issues
// - does not free last block, even if it is empty
// - not an issue, but obvously won't free first block
uint_fast8_t deBloat (memblock *hd) {

    if (hd == NULL) return 0;
    memblock *prev = NULL, *cur = hd;
    // iterate through blocks
    uint_fast8_t blocksDeleted = 0;
    while (cur->pNext != NULL) {
        uint_fast8_t empty = 1;
        for (uint64_t i = 0; i < BLOCK_SIZE; i++) {
            if (cur->mapbits[i]) {
                empty = 0;
                break;
            }
        }
        // free block
        if(empty) {
            if (prev != NULL) prev->pNext = cur->pNext;
            free (cur);
            blocksDeleted++;
        }
        prev = cur;
        cur = cur->pNext;
    }
    return blocksDeleted;
}

uint_fast8_t dumpMap (memblock *hd) {

    while (1) {
        printf ("Memblock index: %d\n\t _________________________________________________________________\n", hd->blockid);
        for (uint_fast8_t y = 0; y < BLOCK_SIZE; y++) { // fixme
            b64 parsingBit = hd->mapbits[y];
            printf ("%d\t| ", y);
            char rowbuffer[65];
            rowbuffer[64] = '\0';
            for (uint_fast8_t x = 0; x < 64; x++) {
                if (parsingBit & 0x1) rowbuffer[63 - x] = '1';
                else rowbuffer[63 - x] = '0';
                parsingBit = parsingBit >> 1;
            }
            printf ("%s\n", rowbuffer);
        }
        printf ("\n");
        if (hd->pNext != NULL) hd = hd->pNext;
        else break;
    }
    return 0;
}
