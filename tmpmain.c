#include <stdio.h>
#include "spirit_memory.h"
#include "spirit_memmap.h"

#define new(type) (type*)malloc(sizeof(type))

int main () {

    memblock testblock = {};
    testblock.mapbits[0] = 0b000010110111; // index 9 for 3

    unsigned x = findPtr (&testblock, 1);
    printf ("%u\n", x);
    //testblock.mapbits[2] = 0xf0;
}
