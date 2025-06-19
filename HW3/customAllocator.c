#include "customAllocator.h"

void* customMalloc(size_t size){
    if (!g_heap) heapCreate();
    size_t round_size = ALIGN_TO_MULT_OF_4(size + OVERHEAD_SIZE);
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
        alloc_block->next = NULL;
        alloc_block->prev = g_heap->lastBlock;
        alloc_block->free = false;
        alloc_block->size = round_size;
        alloc_block->data = (void*)((char*)alloc_block + OVERHEAD_SIZE);
        
        g_heap->lastBlock = alloc_block;
        if (g_heap->firstBlock == NULL) {
            g_heap->firstBlock = alloc_block;
        }
        return alloc_block->data;
    }
}

void customFree(void* ptr);
void* customCalloc(size_t nmemb, size_t size);
void* customRealloc(void* ptr, size_t size);