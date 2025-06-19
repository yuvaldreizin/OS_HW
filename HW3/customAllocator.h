#ifndef CUSTOM_ALLOCATOR
#define CUSTOM_ALLOCATOR

#include <unistd.h> 
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

/*=============================================================================
* Do no edit lines below!
=============================================================================*/
#include <stddef.h> //for size_t

void* customMalloc(size_t size);
void customFree(void* ptr);
void* customCalloc(size_t nmemb, size_t size);
void* customRealloc(void* ptr, size_t size);

/*=============================================================================
* Do no edit lines above!
=============================================================================*/
// Heap creation and destruction functions - 
void heapCreate();
void heapKill();

/*=============================================================================
* If writing bonus - uncomment lines below
=============================================================================*/
// #ifndef BONUS
// #define BONUS
// #endif
// void* customMTMalloc(size_t size);
// void customMTFree(void* ptr);
// void* customMTCalloc(size_t nmemb, size_t size);
// void* customMTRealloc(void* ptr, size_t size);

// void heapCreate();
// void heapKill();

/*=============================================================================
* Defines
=============================================================================*/
#define SBRK_FAIL (void*)(-1)
#define ALIGN_TO_MULT_OF_4(x) (((((x) - 1) >> 2) << 2) + 4)
#define OVERHEAD_SIZE sizeof(struct Block)

/*=============================================================================
* Block
=============================================================================*/
// Suggestion for block usage - feel free to change this
struct Block {
    block_t next;
    block_t prev;
    // block_t nextFree;
    void* data; 
    size_t size;
    bool free;
};
typedef struct Block* block_t;

struct Heap {
    block_t firstBlock;
    block_t lastBlock;
    // size_t totalSize;
};
typedef struct Heap* heap_t;

heap_t g_heap;

block_t checkPtr(void* ptr);
void out_of_memory();

#endif // CUSTOM_ALLOCATOR