#include "customAllocator.h"  // Replace with "customAllocator.h" if you have a separate header
#include <iostream>
#include <cstring>

using namespace std;

int main() {
    heapCreate();  // Optional initialization function

    cout << "--- Testing customMalloc ---" << endl;
    int* a = (int*)customMalloc(sizeof(int) * 4);
    if (a) {
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        a[3] = 4;
        for (int i = 0; i < 4; i++) {
            cout << "a[" << i << "] = " << a[i] << endl;
        }
    }

    cout << "--- Testing customCalloc ---" << endl;
    int* b = (int*)customCalloc(5, sizeof(int));
    if (b) {
        for (int i = 0; i < 5; i++) {
            cout << "b[" << i << "] = " << b[i] << endl;
        }
    }

    cout << "--- Testing customRealloc (expand) ---" << endl;
    int* c = (int*)customRealloc(a, sizeof(int) * 8);
    if (c) {
        for (int i = 0; i < 4; i++) {
            cout << "c[" << i << "] = " << c[i] << endl;
        }
        c[4] = 10;
        cout << "c[4] = " << c[4] << endl;
    }

    cout << "--- Testing customFree ---" << endl;
    customFree(b);
    customFree(c);

    cout << "--- Testing customRealloc (shrink) ---" << endl;
    int* shrink = (int*)customMalloc(sizeof(int) * 6);
    if (shrink) {
        for (int i = 0; i < 6; i++) {
            shrink[i] = i + 100;
        }
        shrink = (int*)customRealloc(shrink, sizeof(int) * 3);
        for (int i = 0; i < 3; i++) {
            cout << "shrink[" << i << "] = " << shrink[i] << endl;
        }
        customFree(shrink);
    }

    cout << "--- Testing reuse of freed block ---" << endl;
    int* block1 = (int*)customMalloc(sizeof(int) * 2);
    customFree(block1);
    int* block2 = (int*)customMalloc(sizeof(int) * 2);
    if (block1 == block2) {
        cout << "Block was reused (same address)" << endl;
    } else {
        cout << "Block was not reused" << endl;
    }
    customFree(block2);

    cout << "--- Testing customFree(nullptr) ---" << endl;
    customFree(nullptr);  // Should print an error message

    cout << "--- Testing customFree on invalid pointer ---" << endl;
    int invalid;
    customFree(&invalid);  // Should print an error message

    cout << "--- Testing customRealloc on invalid pointer ---" << endl;
    int stackVar;
    void* result = customRealloc(&stackVar, sizeof(int) * 2);
    if (result == nullptr) {
        cout << "customRealloc rejected non-heap pointer as expected." << endl;
    }

    heapKill();  // Optional cleanup

    return 0;
}

