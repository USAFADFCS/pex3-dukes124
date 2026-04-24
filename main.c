/** main.c
 * ===========================================================
 * Name: Samuel Lavoie, 23 April 2026
 * Section: CS483 / M4
 * Project: PEX3 - Page Replacement Simulator
 * Purpose: Reads a BYU binary memory trace file and simulates
 *          LRU page replacement to measure fault rates across
 *          varying frame allocations.
 * Documentation: Used https://www.geeksforgeeks.org/dsa/program-for-least-recently-used-lru-page-replacement-algorithm/
 * to help with algorithim implementation. Used CS220 DLL code as a reference for the doubly linked list implementation.
 * =========================================================== */
#include <stdio.h>
#include <stdlib.h>
#include "byutr.h"
#include "pagequeue.h"

#define PROGRESS_INTERVAL 100000  // print status every N accesses

int main(int argc, char **argv) {
    FILE *ifp = NULL;
    unsigned long numAccesses = 0;
    p2AddrTr traceRecord;

    // Validate command-line arguments: trace_file frame_size
    if (argc != 3) {
        fprintf(stderr, "usage: %s input_byutr_file frame_size\n", argv[0]);
        fprintf(stderr, "\nframe_size:\n\t1: 512 bytes\n\t2: 1KB\n\t3: 2KB\n\t4: 4KB\n");
        exit(1);
    }

    // Open the binary trace file
    ifp = fopen(argv[1], "rb");
    if (ifp == NULL) {
        fprintf(stderr, "cannot open %s for reading\n", argv[1]);
        exit(1);
    }

    // Parse and validate frame size selection
    int menuOption = atoi(argv[2]);
    if (menuOption < 1 || menuOption > 4) {
        fprintf(stderr, "invalid frame size option: %s (must be 1-4)\n", argv[2]);
        fclose(ifp);
        exit(1);
    }

    // Map menu option to page geometry
    int offsetBits = 0;
    int maxFrames = 0;
    switch (menuOption) {
        case 1:
            offsetBits = 9;   // 512-byte pages
            maxFrames = 8192;
            break;
        case 2:
            offsetBits = 10;  // 1KB pages
            maxFrames = 4096;
            break;
        case 3:
            offsetBits = 11;  // 2KB pages
            maxFrames = 2048;
            break;
        case 4:
            offsetBits = 12;  // 4KB pages
            maxFrames = 1024;
            break;
    }

    fprintf(stderr, "Frame size option %d: %d offset bits, %d max frames, algorithm=LRU\n",
            menuOption, offsetBits, maxFrames);

    PageQueue *pq = pqInit(maxFrames);
    if (pq == NULL) {
        fprintf(stderr, "failed to initialize page queue\n");
        fclose(ifp);
        exit(1);
    }

    unsigned long *faults = calloc(maxFrames + 1, sizeof(unsigned long)); // initlized with +1 to help with indexing
    if (faults == NULL) { 
        fprintf(stderr, "failed to allocate faults array\n");
        pqFree(pq);
        fclose(ifp);
        exit(1);
    }

    // Process each memory access from the trace file
    while (!feof(ifp)) {
        fread(&traceRecord, sizeof(p2AddrTr), 1, ifp);

        // Extract page number by shifting off the offset bits
        unsigned long pageNum = traceRecord.addr >> offsetBits;
        numAccesses++;

        // Print progress indicator to stderr every PROGRESS_INTERVAL accesses
        // (also prints the last page number seen — useful for early debugging)
        if ((numAccesses % PROGRESS_INTERVAL) == 0) {
            fprintf(stderr, "%lu samples read, last page: %lu\r", numAccesses, pageNum);
        }

        long depth = pqAccess(pq, pageNum);

        if (depth == -1) {
            // Miss, page was NOT in the queue (fault for ALL frame counts)
            for (int f = 1; f <= maxFrames; f++) {
                faults[f]++;
            }
        } else {
            // Hit, page was at depth d from the MRU end
            for (int f = 1; f <= depth; f++) {
                faults[f]++;
            }
        }

    }

    fprintf(stderr, "\n%lu total accesses processed\n", numAccesses);

    // Output CSV results to stdout (redirect with > to create a .csv file)
    printf("Total Accesses:,%lu\n", numAccesses);
    printf("Frames,Missees,Miss Rate\n");

    for (int f = 1; f <= maxFrames; f++) {
        printf("%d,%lu,%f\n", f, faults[f],
               (double)faults[f] / (double)numAccesses);
    }

    pqFree(pq);
    free(faults);
    fclose(ifp);

    return 0;
}
