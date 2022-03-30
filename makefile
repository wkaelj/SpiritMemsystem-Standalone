CC = cc 
CFLAGS = -Wall -g
LFLAGS =

BUILD = $(CURDIR)/build

default: run

debug: test
	gdb -tui $(BUILD)/test.x86_64

run: all test
	$(CURDIR)/build/test.x86_64

test: all tmpmain.c
	$(CC) $(CFLAGS) tmpmain.c $(BUILD)/libspmemory.a -o $(BUILD)/test.x86_64

all: memmap memory
	mkdir -p $(BUILD)/
	ar rcs $(BUILD)/libspmemory.a $(BUILD)/spirit_memmap.o $(BUILD)/spirit_memory.o

memmap: spirit_memmap.c spirit_memmap.h
	$(CC) -c $(CFLAGS) $(LFLAGS) spirit_memmap.c -o $(BUILD)/spirit_memmap.o

memory: spirit_memory.c spirit_memory.h
	$(CC) -c $(CFLAGS) $(LFLAGS) spirit_memory.c -o $(BUILD)/spirit_memory.o
