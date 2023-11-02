#include "assignment7.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define SORT_THRESHOLD      40

typedef struct _sortParams {
    char** array;
    int left;
    int right;
    int threads_left;
} SortParams;

static int maximumThreads;              /* maximum # of threads to be used */

/* This is an implementation of insert sort, which although it is */
/* n-squared, is faster at sorting short lists than quick sort,   */
/* due to its lack of recursive procedure call overhead.          */

static void insertSort(char** array, int left, int right) {
    int i, j;
    for (i = left + 1; i <= right; i++) {
        char* pivot = array[i];
        j = i - 1;
        while (j >= left && (strcmp(array[j],pivot) > 0)) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = pivot;
    }
}

/* Recursive quick sort, but with a provision to use */
/* insert sort when the range gets small.            */

static void quickSort(void* p) {
    SortParams* params = (SortParams*) p;
    char** array = params->array;
    int left = params->left;
    int right = params->right;
    int i = left, j = right;
    int threads_left = params->threads_left;

    pthread_t thread1;
    pthread_t thread2;


    
    if (j - i > SORT_THRESHOLD) {           /* if the sort range is substantial, use quick sort */

        int m = (i + j) >> 1;               /* pick pivot as median of         */
        char* temp, *pivot;                 /* first, last and middle elements */
        if (strcmp(array[i],array[m]) > 0) {
            temp = array[i]; array[i] = array[m]; array[m] = temp;
        }
        if (strcmp(array[m],array[j]) > 0) {
            temp = array[m]; array[m] = array[j]; array[j] = temp;
            if (strcmp(array[i],array[m]) > 0) {
                temp = array[i]; array[i] = array[m]; array[m] = temp;
            }
        }
        pivot = array[m];

        for (;;) {
            while (strcmp(array[i],pivot) < 0) i++; /* move i down to first element greater than or equal to pivot */
            while (strcmp(array[j],pivot) > 0) j--; /* move j up to first element less than or equal to pivot      */
            if (i < j) {
                char* temp = array[i];      /* if i and j have not passed each other */
                array[i++] = array[j];      /* swap their respective elements and    */
                array[j--] = temp;          /* advance both i and j                  */
            } else if (i == j) {
                i++; j--;
            } else break;                   /* if i > j, this partitioning is done  */
        }

        // check if we have available threads to distrisbute computation
        if (threads_left > 0 ) {

            // we can make more threads, allocate parameters on the heap
            SortParams *first = malloc(sizeof(SortParams)); 
            first->array = array; 
            first->left = left; 
            first->right = j;
            // decrement available amount of threads, this wont run in parallel so we dont need a mutex
            first->threads_left = threads_left - 1;

            // run quicksort on the other partition in parallel, we dont need the heap
            SortParams second; second.array = array; second.left = i; second.right = right;
            second.threads_left = 0;

            // create a new thread
            pthread_create(&thread1, NULL, (void *) quickSort, (void *) first);

            quickSort(&second);                 /* sort the right partition */
            pthread_join(thread1, NULL);
            free(first);
            return;
        }

        // no more available threads, run how normal quicksort should run
        SortParams first;  first.array = array; first.left = left; first.right = j;
        first.threads_left = 0;

        quickSort(&first);                  /* sort the left partition  */
        
        SortParams second; second.array = array; second.left = i; second.right = right;
        second.threads_left = 0;
        
        quickSort(&second);                 /* sort the right partition */
                
    } else insertSort(array,i,j);           /* for a small range use insert sort */
}

/* user interface routine to set the number of threads sortT is permitted to use */

void setSortThreads(int count) {
    maximumThreads = count;
}

/* user callable sort procedure, sorts array of count strings, beginning at address array */

void sortThreaded(char** array, unsigned int count) {
    SortParams parameters;
    parameters.array = array; parameters.left = 0; parameters.right = count - 1;
    // set initial maximumThreads, dont need mutex since we never access this variable in parallel
    parameters.threads_left = maximumThreads;
    quickSort(&parameters);
}
