/*****************************************************************************
 * life.c
 * The original sequential implementation resides here.
 * Do not modify this file, but you are encouraged to borrow from it
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

void *process(void *args);
//class argument;

typedef struct argument {
        char* outboard;
        char* inboard;
        int nrows;
        int ncols;
        int start;
        int end;
}argument;

unsigned num_threads = 4;

char*
game_of_life (char* outboard, 
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{ 
     int i,j,curgen;
     int start_row = 0;
     int num_rows_per_thread = nrows / num_threads;
     int end_row = num_rows_per_thread;
     
     pthread_t threads[num_threads];
     argument *args = malloc(num_threads * sizeof(argument));
 
     for (i=0; i<num_threads; i++){
          args[i].start = start_row;
          args[i].end = end_row - 1;
          args[i].nrows = nrows;
          args[i].ncols = ncols;
          
          start_row += num_rows_per_thread;
          end_row += num_rows_per_thread;
          
     }
     for (curgen=0; curgen<gens_max;curgen++){
          for (i=0; i<num_threads; i++){
               args[i].outboard = outboard;
               args[i].inboard = inboard;
               pthread_create(&threads[i], NULL, process, (void*)&args[i]);
          }
     
          for (j=0; i<num_threads; i++){
               pthread_join(threads[j], NULL);
          }
          
          SWAP_BOARDS( outboard, inboard );
      }

     free(args);
     return inboard;
}


void *process(void *args)
{
    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */
    
    int i, j;
    argument *val = (argument *) args;
    char* outboard = val->outboard;
    char* inboard = val->inboard;
    int nrows = val->nrows;
    int ncols = val->ncols;
    int start = val->start;
    int end = val->end;
    int LDA = end - start +1;
    
        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
        for (i = start; i < end; i++)
        {
            for (j = 0; j < ncols; j++)
            {
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);

                const char neighbor_count = 
                    BOARD (inboard, inorth, jwest) + 
                    BOARD (inboard, inorth, j) + 
                    BOARD (inboard, inorth, jeast) + 
                    BOARD (inboard, i, jwest) +
                    BOARD (inboard, i, jeast) + 
                    BOARD (inboard, isouth, jwest) +
                    BOARD (inboard, isouth, j) + 
                    BOARD (inboard, isouth, jeast);

                BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));

            }
        }
        return inboard;

    }
    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */


