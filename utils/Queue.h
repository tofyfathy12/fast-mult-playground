#include <stdio.h>
#include <stdlib.h>

#define False 0
#define True 1
typedef unsigned char boolean;

typedef struct nd {
    void* data;
    struct nd* next;
} Node;

typedef struct qu {
    Node* head;
    Node* tail;
    void (*enqueue)(struct qu* self, void* data);
    void* (*dequeue)(struct qu* self);
    void* (*front)(struct qu* self);
    boolean (*isEmpty)(struct qu* self);
    void (*print)(struct qu* self);
    size_t size;
} Queue;

void destroy_queue(Queue* q);
Queue* construct_queue();
void print_int_queue(Queue* self);
void print_str_queue(Queue* self);
void enqueue(Queue* self, void* data);
void* dequeue(Queue* self);
void* front(Queue* self);
boolean isEmpty(Queue* self);
Node* construct_node(void* data);

void destroy_queue(Queue* q) {
    if (q == NULL) return;
    Node* prev = NULL;
    for (Node* iter = q->head; iter != NULL; iter = iter->next) {
        free(iter->data);
        if (prev != NULL) {
            free(prev);
        }
        prev = iter;
    }
    free(prev);
    free(q);
}

Queue* construct_queue() {
    Queue* new_queue = (Queue*)malloc(sizeof(Queue));
    if (new_queue == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for queue\n");
        return NULL;
    }

    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->size = 0;
    new_queue->enqueue = enqueue;
    new_queue->dequeue = dequeue;
    new_queue->front = front;
    new_queue->isEmpty = isEmpty;

    return new_queue;
}

void print_int_queue(Queue* self) {
    printf("[");
    for (Node* iter = self->head; iter != NULL; iter = iter->next) {
        printf("%d", *(int*)iter->data);
        if (iter->next != NULL)
            printf(", ");
    }
    printf("]\n");
}

void print_str_queue(Queue* self) {
    printf("[");
    for (Node* iter = self->head; iter != NULL; iter = iter->next) {
        printf("%s", (char*)iter->data);
        if (iter->next != NULL)
            printf(", ");
    }
    printf("]\n");
}

void enqueue(Queue* self, void* data) { //add to tail and advance tail pointer
    Node* new_node = construct_node(data);
    if (self->isEmpty(self)) {
        self->head = new_node;
        self->tail = new_node;
    }
    else {
        self->tail->next = new_node;
        self->tail = new_node;
    }
    self->size++;
}

void* dequeue(Queue* self) { //remove from head and advance head pointer
    if (self->isEmpty(self)) {
        fprintf(stderr, "Error: Can't use dequeue() on Empty Queue\n");
        return NULL;
    }
    Node* removed_node = self->head;
    self->head = self->head->next;
    if (self->head == NULL) {
        self->tail = NULL;
    }
    self->size--;
    void* returned_data = removed_node->data;
    free(removed_node);
    return returned_data;
}

void* front(Queue* self) { //look at the item to be removed next
    if (self->isEmpty(self)) {
        fprintf(stderr, "Error: Can't use front() on Empty Queue\n");
        return NULL;
    }
    return self->head->data;
}

boolean isEmpty(Queue* self) {
    return (self->size == 0);
}

Node* construct_node(void* data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for node\n");
        return NULL;
    }

    new_node->data = data;
    new_node->next = NULL;

    return new_node;
}

