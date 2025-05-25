#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>

// Node structure
typedef struct Node {
    void *data;
    struct Node *next;
} Node;

// Linked list structure
typedef struct LinkedList {
    Node *head;
} LinkedList;


LinkedList* linked_list_init();
void linked_list_add(LinkedList *list, void *data);
void linked_list_remove(LinkedList *list, void *data);
void linked_list_sorted_insert(LinkedList *list, void *data, int (*cmp_func)(void *, void *));
void linked_list_foreach(LinkedList *list, void (*func)(void *));
void linked_list_free(LinkedList *list, void (*free_func)(void *));


#endif // LINKED_LIST_H