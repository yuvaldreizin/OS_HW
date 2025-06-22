#include "customAllocator.h"

heap_t g_heap = NULL; // global heap pointer


void* customMalloc(size_t size){
    // if (size == 0) return NULL; // decide if we want to allow 0 size allocations
    if (!g_heap) heapCreate();
    size_t round_size = ALIGN_TO_MULT_OF_4(size) + OVERHEAD_SIZE;
    block_t block = g_heap->firstBlock;
    block_t best_match = NULL;
    // find best match from existing blocks
    while (block != NULL){
        if (block->free && block->size >= round_size) {
            if (best_match == NULL || block->size < best_match->size) {
                best_match = block;
            }
        }
        block = block->next;
    }
    if (best_match != NULL) {   // found match
        best_match->free = false;  
        return best_match->data; 
    } else {    // no match, create new block
        block_t alloc_block = (block_t)(sbrk(round_size));
        if (alloc_block == SBRK_FAIL && errno == ENOMEM) {
            out_of_memory();
        }
        alloc_block->next = NULL;
        alloc_block->free = false;
        alloc_block->size = round_size - OVERHEAD_SIZE; // size without overhead
        if (g_heap->firstBlock != NULL) // if there are existing blocks
        {
            g_heap->lastBlock->next = alloc_block;
            alloc_block->prev = g_heap->lastBlock;
            g_heap->lastBlock = alloc_block;
        }
        else // if no existing blocks, set first and last to the new block
        {
            g_heap->firstBlock = alloc_block;
            g_heap->lastBlock = alloc_block;
            alloc_block->prev = NULL;
            alloc_block->next = NULL;
        }
        if (size == 0 ) { 
            alloc_block->data = NULL; // won't be useful, will be dealt in free
        } else {
            alloc_block->data = (void*)((char*)alloc_block + OVERHEAD_SIZE);
        }
        
        // g_heap->lastBlock = alloc_block;
        // if (g_heap->firstBlock == NULL) {
        //     g_heap->firstBlock = alloc_block;
        // }
        return alloc_block->data;
    }
}

void customFree(void* ptr){
    int result = checkPtr(ptr);
    if (result == 2){
        printf("<free error>: passed null pointer\n");
        return;
    }
    if (result == 1) {
        printf("<free error>: passed non-heap pointer\n");
        return;
    }
    // result == 0, ptr is valid heap pointer
    block_t block = (block_t)((char*)ptr - OVERHEAD_SIZE);
    block->free = true;
    while (block->next != NULL && block->next->free) { // merge with next free block
        block_t next_block = block->next;
        block->size += next_block->size + OVERHEAD_SIZE; // add size of next block and overhead
        block->next = next_block->next;
        if (next_block->next != NULL) {
            next_block->next->prev = block; // update prev pointer of next block
        } else {
            g_heap->lastBlock = block; // update last block if needed
        }
    }
    while (block->prev != NULL && block->prev->free) { // merge with previous free block
        block_t prev_block = block->prev;
        prev_block->size += block->size + OVERHEAD_SIZE; // add size of current block and overhead
        prev_block->next = block->next;
        if (block->next != NULL) {
            block->next->prev = prev_block; // update prev pointer of next block
        } else {
            g_heap->lastBlock = prev_block; // update last block if needed
        }
        block = prev_block; // move to previous block
    }
    if (block == g_heap->lastBlock) { // if last block was freed, remove it from heap
        g_heap->lastBlock = block->prev;
        if (g_heap->lastBlock == NULL) {
            g_heap->firstBlock = NULL; // if no blocks left, set first block to NULL
        } else {
            g_heap->lastBlock->next = NULL; // update next pointer of last block
        }
        // free the last block memory
        if (brk(block) == -1 && errno == ENOMEM) {
            out_of_memory();
        }
    }
    return;
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
        if (size_diff > OVERHEAD_SIZE) { // can shrink
            block_t new_block = (block_t)((char*)block + OVERHEAD_SIZE + size);
            new_block->next = block->next;
            new_block->prev = block;
            new_block->free = true;
            new_block->size = size_diff - OVERHEAD_SIZE; 
            // if (new_block->size == 0 ) {
            //     new_block->data = NULL; // won't be useful, will be dealt in free
            // } else {
            new_block->data = (void*)((char*)new_block + OVERHEAD_SIZE);
            // }

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

// for debug

void print_heap_blocks(const char* msg) {
    printf("=== Heap Blocks: %s ===\n", msg ? msg : "");
    if (!g_heap) {
        printf("Heap not initialized.\n");
        return;
    }
    block_t block = g_heap->firstBlock;
    int idx = 0;
    while (block) {
        printf("  Block %d: addr=%p, size=%zu, free=%s, data=%p, prev=%p, next=%p\n",
            idx,
            (void*)block,
            block->size,
            block->free ? "yes" : "no",
            block->data,
            (void*)block->prev,
            (void*)block->next
        );
        block = block->next;
        idx++;
    }
    if (idx == 0) {
        printf("  (no blocks in heap)\n");
    }
    else {
        printf("  first block: %p\n", (void*)g_heap->firstBlock);
        printf("  last block: %p\n", (void*)g_heap->lastBlock);
    }
    printf("============================\n");
}