/** pagequeue.c
 * ===========================================================
 * Name: _______________________, __ ___ 2026
 * Section: CS483 / ____
 * Project: PEX3 - Page Replacement Simulator
 * Purpose: Implementation of the PageQueue ADT — a doubly-linked
 *          list for LRU page replacement.
 *          Head = LRU (eviction end), Tail = MRU end.
 * =========================================================== */
#include <stdio.h>
#include <stdlib.h>

#include "pagequeue.h"

/**
 * @brief Create and initialize a page queue with a given capacity
 */
PageQueue *pqInit(unsigned int maxSize) {
    PageQueue *pq = (PageQueue *)malloc(sizeof(PageQueue));
    if (pq) {
        pq->head = NULL;
        pq->tail = NULL;
        pq->size = 0;
        pq->maxSize = maxSize;
    }
    return pq;
}

/**
 * @brief Access a page in the queue (simulates a memory reference)
 */
long pqAccess(PageQueue *pq, unsigned long pageNum) {
    PqNode *current = pq->tail; // start at end
    long depth = 0;
    while (current != NULL && current->pageNum != pageNum) { // scan towards head
        current = current->prev;
        depth++;
    }
    //HIT
    if (current != NULL) { // page found
        if (depth == 0) {
            return 0;  // mru already, dont move
        }

        if (current->prev != NULL) { // not head, update prev next
            current->prev->next = current->next;
        } else {
            pq->head = current->next;  // current is head
        }
        current->next->prev = current->prev;  // current not tail

        current->prev = pq->tail; // current becomes tail 
        current->next = NULL; 
        pq->tail->next = current;
        pq->tail = current;

        return depth;
    }

    // MISS
    // page not in queue, allocate node and append at tail
    PqNode *newNode = (PqNode *)malloc(sizeof(PqNode));
    newNode->pageNum = pageNum;
    newNode->next = NULL;
    newNode->prev = pq->tail;
    if (pq->tail != NULL) { // queue not empty, link tail to new node
        pq->tail->next = newNode;
    } else {
        pq->head = newNode;  // queue was empty
    }
    pq->tail = newNode;
    pq->size++;

    if (pq->size > pq->maxSize) { // evict LRU page at head
        PqNode *victim = pq->head;
        pq->head = victim->next;
        if (pq->head != NULL) { 
            pq->head->prev = NULL;
        } else {
            pq->tail = NULL;  // empty queue after evic
        }
        free(victim);
        pq->size--;
    }

    return -1;
}

/**
 * @brief Free all nodes in the queue and reset it to empty
 */
void pqFree(PageQueue *pq) {
    PqNode *current = pq->head;
    PqNode *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(pq);
}

/**
 * @brief Print queue contents to stderr for debugging
 */
void pqPrint(PageQueue *pq) {
    // TODO (optional): Print each page number from head to tail,
    //                  marking which is head and which is tail.
    //                  Useful for desk-checking small traces.
}
