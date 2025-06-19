#include <iostream>
#include <thread>
#include <vector>
#include <cassert>
#include <cstring>
#include "customAllocator.h"

// Thread test: calloc, verify zero init, write ID, realloc, verify data, free
void threadFunc(int tid) {
    std::cout << "[Thread " << tid << "] started\n";

    // Allocate memory
    int* arr = (int*)customMTCalloc(128, sizeof(int));
    assert(arr != nullptr);

    // Check zero initialization
    for (int i = 0; i < 128; ++i) {
        assert(arr[i] == 0);
    }

    // Write thread ID
    for (int i = 0; i < 128; ++i) {
        arr[i] = tid;
    }

    // Verify data
    for (int i = 0; i < 128; ++i) {
        assert(arr[i] == tid);
    }

    // Reallocate to larger size
    arr = (int*)customMTRealloc(arr, 256 * sizeof(int));
    assert(arr != nullptr);

    // Continue writing
    for (int i = 128; i < 256; ++i) {
        arr[i] = tid;
    }

    // Final verify
    for (int i = 0; i < 256; ++i) {
        assert(arr[i] == tid);
    }

    // Free
    customMTFree(arr);

    std::cout << "[Thread " << tid << "] finished\n";
}

int main() {
    heapCreate(); // Initialize allocator

    const int NUM_THREADS = 8;
    std::vector<std::thread> threads;

    // Launch threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(threadFunc, i);
    }

    // Join threads
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "All threads finished successfully.\n";

    // Additional error case: double free and invalid pointer
    int* fake = new int;
    customMTFree(fake);  // should print error
    delete fake;

    void* p = customMTMalloc(32);
    customMTFree(p);
    customMTFree(p);     // should print error

    heapKill(); // Clean up
    return 0;
}
