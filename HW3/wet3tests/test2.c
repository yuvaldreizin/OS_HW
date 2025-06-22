#include "customAllocator.h"  // Replace with "customAllocator.h" if you have a separate header
#include <stdio.h>
#include <string.h>

int main() {
    heapCreate();  // Optional initialization function

    printf("--- Testing customMalloc ---\n");
    int* a = (int*)customMalloc(sizeof(int) * 4);
    if (a) {
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        a[3] = 4;
        for (int i = 0; i < 4; i++) {
            printf("a[%d] = %d\n", i, a[i]);
        }
    }
    printf("%ld\n", OVERHEAD_SIZE);

    printf("--- Testing customCalloc ---\n");
    int* b = (int*)customCalloc(5, sizeof(int));
    if (b) {
        for (int i = 0; i < 5; i++) {
            printf("b[%d] = %d\n", i, b[i]);
        }
    }

    printf("--- Testing customRealloc (expand) ---\n");
    int* c = (int*)customRealloc(a, sizeof(int) * 8);
    if (c) {
        for (int i = 0; i < 4; i++) {
            printf("c[%d] = %d\n", i, c[i]);
        }
        c[4] = 10;
        printf("c[4] = %d\n", c[4]);
    }
    printf("--- Testing customFree ---\n");
    customFree(b);
    customFree(c);

    printf("--- Testing customRealloc (shrink) ---\n");
    int* shrink = (int*)customMalloc(sizeof(int) * 6);
    if (shrink) {
        for (int i = 0; i < 6; i++) {
            shrink[i] = i + 100;
        }
        shrink = (int*)customRealloc(shrink, sizeof(int) * 3);
        for (int i = 0; i < 3; i++) {
            printf("shrink[%d] = %d\n", i, shrink[i]);
        }
        customFree(shrink);
    }

    printf("--- Testing reuse of freed block ---\n");
    int* block1 = (int*)customMalloc(sizeof(int) * 2);
    customFree(block1);
    int* block2 = (int*)customMalloc(sizeof(int) * 2);
    if (block1 == block2) {
        printf("Block was reused (same address)\n");
    } else {
        printf("Block was not reused\n");
    }
    customFree(block2);

    printf("--- Testing customFree(NULL) ---\n");
    customFree(NULL);  // Should print an error message

    printf("--- Testing customFree on invalid pointer ---\n");
    int invalid;
    customFree(&invalid);  // Should print an error message

    printf("--- Testing customRealloc on invalid pointer ---\n");
    int stackVar;
    void* result = customRealloc(&stackVar, sizeof(int) * 2);
    if (result == NULL) {
        printf("customRealloc rejected non-heap pointer as expected.\n");
    }

    heapKill();  // Optional cleanup

    return 0;
}