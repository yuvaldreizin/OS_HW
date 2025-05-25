#include "linked_list.h"

LinkedList* linked_list_init() {
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    if (!list) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    list->head = NULL;
    return list;
}

// Add an element to the front of the list
void linked_list_add(LinkedList *list, void *data) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    new_node->data = data;
    new_node->next = list->head;
    list->head = new_node;
}

// Remove an element from the list
void linked_list_remove(LinkedList *list, void *data) {
    Node *current = list->head;
    Node *prev = NULL;

    while (current) {
        if (current->data == data) {
            if (prev) {
                prev->next = current->next;
            } else {
                list->head = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Insert an element into the list in sorted order
void linked_list_sorted_insert(LinkedList *list, void *data, int (*cmp_func)(void *, void *)) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    new_node->data = data;
    new_node->next = NULL;

    // 1st iteration
    if (!list->head || cmp_func(data, list->head->data) <= 0) {
        // Insert at the head if the list is empty or the new data is smaller
        new_node->next = list->head;
        list->head = new_node;
        return;
    }

    Node *current = list->head;
    while (current->next && cmp_func(data, current->next->data) > 0) {
        current = current->next;
    }

    new_node->next = current->next;
    current->next = new_node;
}

// Iterate over the list
void linked_list_foreach(LinkedList *list, void (*func)(void *)) {
    Node *current = list->head;
    while (current) {
        func(current->data);
        current = current->next;
    }
}

// Free the entire list
void linked_list_free(LinkedList *list, void (*free_func)(void *)) {
    Node *current = list->head;
    while (current) {
        Node *next = current->next;
        if (free_func) {
            free_func(current->data);
        }
        free(current);
        current = next;
    }
    free(list);
}