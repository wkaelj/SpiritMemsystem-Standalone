#include <stdio.h>
#include "spirit_memory.h"
#include "spirit_memmap.h"

#define new(type) (type*)malloc(sizeof(type))

int main () {

    memblock testblock = {};
    testblock.mapbits[0] = B64_MAX; // index 9 for 3
    testblock.mapbits[1] = 0b101;

    unsigned x = findPtr (&testblock, 3);
    printf ("%u\n", x);
    //testblock.mapbits[2] = 0xf0;
}
