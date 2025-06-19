#include "customAllocator.h"

void* customMalloc(size_t size){
    // if (size == 0) return NULL; // decide if we want to allow 0 size allocations
    if (!g_heap) heapCreate();
    size_t round_size = ALIGN_TO_MULT_OF_4(size) + OVERHEAD_SIZE;
    block_t block = g_heap->firstBlock;
    block_t best_match = NULL;
    // find best match from existing blocks
    while (block != NULL){
        if (block->free && block->size >= round_size) {
            if (best_match != NULL || block->size < best_match->size) {
                best_match = block;
            } else if (best_match == NULL) {
                best_match = block;
            }
        }
        block = block->next;
    }
    
    if (best_match != NULL) {   // found match
        best_match->free = false;  
        return best_match->data; 
    } else {    // no match, create new block
        block_t alloc_block = (block_t)sbrk(round_size);
        if (alloc_block == SBRK_FAIL && errno == ENOMEM) {
            out_of_memory();
        }
        alloc_block->next = NULL;
        alloc_block->prev = g_heap->lastBlock;
        alloc_block->free = false;
        alloc_block->size = round_size;
        if (size == 0 ) { 
            alloc_block->data = NULL; // won't be useful, will be dealt in free
        } else {
            alloc_block->data = (void*)((char*)alloc_block + OVERHEAD_SIZE);
        }
        
        g_heap->lastBlock = alloc_block;
        if (g_heap->firstBlock == NULL) {
            g_heap->firstBlock = alloc_block;
        }
        return alloc_block->data;
    }
}

void customFree(void* ptr){

}

void* customCalloc(size_t nmemb, size_t size){
    // memory for an array, so continuouse
    void* alloc_mem = customMalloc(nmemb * size); // already aligned to 4
    for (size_t i = 0; i < nmemb * size; i++) {
        ((char*)alloc_mem)[i] = 0;
    }
    return alloc_mem;
}

void* customRealloc(void* ptr, size_t size){
    // ptr check
    if (ptr == NULL) {
        return customMalloc(size);
    } else {
        if (checkPtr(ptr) != 0){
            printf("<realloc error>: passed non-heap pointer\n");
            return NULL;
        }
    }

    block_t block = (block_t)((char*)ptr - OVERHEAD_SIZE);
    size_t old_size = block->size;
    if (size < old_size){
        size_t size_diff = old_size - size;
        if (size_diff >= OVERHEAD_SIZE) { // can shrink
            block_t new_block = (block_t)((char*)block + size);
            new_block->next = block->next;
            new_block->prev = block;
            new_block->free = true;
            new_block->size = size_diff - OVERHEAD_SIZE; 
            if (new_block->size == 0 ) { 
                new_block->data = NULL; // won't be useful, will be dealt in free
            } else {
                new_block->data = (void*)((char*)new_block + OVERHEAD_SIZE);
            }

            block->size = size;
            block->next = new_block;
            return ptr;
        }
    } else if (size != old_size){   // new size is larger, or smaller but can't shrink
        void* new_data = customMalloc(size);
        for (int i = 0; i < old_size && i < size; i++) {
            ((char*)new_data)[i] = ((char*)ptr)[i];
        }
        customFree(ptr);
        return new_data;
    }
    return ptr; // size == old_size
}

int checkPtr(void* ptr) {
    // 0 if correct, 1 if not, 2 if NULL
    if (ptr == NULL) return 2;
    if (!g_heap) return 1; // no heap

    block_t block = g_heap->firstBlock;
    while (block != NULL){
        if (block->data == ptr) return 0; // found
        block = block->next;
    }
    return 1;   // not found
}

void out_of_memory(){
    // TODO add checks and calls when brk returns -1 with no memory errno
    if (g_heap != NULL) {
        brk(g_heap);        // !!! correct free? what of non block memory between blocks?   // releasing memory so no out-of-memory possible
        heapKill(); 
    }
    printf("<sbrk/brk error>: out of memory\n");
    exit(1);
}


void heapCreate()
{
    g_heap = (heap_t)sbrk(sizeof(struct Heap));
    if (g_heap == SBRK_FAIL && errno == ENOMEM) {
        out_of_memory();
    }
    g_heap->firstBlock = NULL;
    g_heap->lastBlock = NULL;
    return;
}
void heapKill()
{
    brk(g_heap);    // releasing memory so no out-of-memory possible
    g_heap = NULL;
    return;
}