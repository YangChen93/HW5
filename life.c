/*****************************************************************************
 * life.c
 * The original sequential implementation resides here.
 * Do not modify this file, but you are encouraged to borrow from it
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define BOARD( __board, __i, __j )  (__board[(__i) + nrows*(__j)])
#define INCRE_COUNT( __board, __i, __j )  (__board[(__i) + nrows*(__j)] ++)
#define DECRE_COUNT( __board, __i, __j )  (__board[(__i) + nrows*(__j)] --)
#define check_status(var) ((var) & (1<<(4))>>4)
#define dead_to_alive(var) (var |=  (1 << (4)))
#define alive_to_dead(var) (var &= ~(1 << (4))) 
#define neighbor_count( __board, __i, __j ) (__board[(__i) + nrows*(__j)] & (char)0x0f  )

void *process(void *args);
void process_first_last(char* outboard, char* inboard, const int nrows, const int ncols, int row_start, int num_rows);
void count_board_init(char* inboard, char* outboard,const int nrows, const int ncols);
void print_board(char* board, const int nrows, const int ncols, char* message);


typedef struct argument {
  char* outboard;
  char* inboard;
  int nrows;
  int ncols;
  int row_start;
  int num_rows;
}argument;


char *game_of_life (char* outboard, char* inboard, const int nrows, const int ncols, const int gens_max) { 

  int num_threads = 4;

  int i,j,curgen;
  int row_start = 0;
  int num_rows_per_thread = nrows / num_threads;

  //char* count_inboard = malloc(nrows * ncols * sizeof(char));
  //char* count_outboard = malloc(nrows * ncols * sizeof(char));

  /* Initialize arguments for threads */
  argument *args = malloc(num_threads * sizeof(argument));
  for (i=0; i<num_threads; i++) {
    args[i].nrows = nrows;
    args[i].ncols = ncols;
    args[i].num_rows = num_rows_per_thread;
    args[i].row_start = row_start;
    row_start += num_rows_per_thread;
  }

  /* Create threads */
  pthread_t threads[num_threads];

  count_board_init(inboard, outboard, nrows, ncols);
  //memcpy(count_outboard, count_inboard, nrows * ncols * sizeof (char));

  for (curgen=0; curgen<gens_max; curgen++) {

    //printf("Gen %d - ", curgen);
    //print_board(inboard, nrows, ncols, "inboard");
    //printf("Gen %d - ", curgen);
    //print_board(count_inboard, nrows, ncols, "count_inboard");

    /* Compute for the first and last row of each block to avoid race condition between threads */
    for (i=0; i<num_threads; i++) {
      process_first_last(outboard, inboard, nrows, ncols, i * num_rows_per_thread, num_rows_per_thread);
    }

    /* Process the rest of the rows */
    for (i=0; i<num_threads; i++) {
      args[i].outboard = outboard;
      args[i].inboard = inboard;
      pthread_create(&threads[i], NULL, process, (void*)&args[i]);
    }

    for (i=0; i<num_threads; i++){
      pthread_join(threads[i], NULL);
    }

    //printf("Gen %d - ", curgen);
    //print_board(outboard, nrows, ncols, "outboard");
    //printf("Gen %d - ", curgen);
    //print_board(count_outboard, nrows, ncols, "count_outboard");
   
    SWAP_BOARDS(outboard, inboard);
  }

  for (i = 0; i<nrows; i++) {
	  for (j = 0; j<ncols; j++) {
		  BOARD(inboard, i, j) = BOARD(inboard, i, j) >> 4;
	  }
  }
  free(args);
  return inboard;
}


void *process(void *args)
{
  int i, j;
  int current_state, neighbor_count, next_state;
  int inorth, isouth, jwest, jeast;
  argument *val = (argument *) args;

  char* outboard = val->outboard;
  char* inboard = val->inboard;

  int row_start = val->row_start + 1;
  int row_end = val->row_start + val->num_rows - 1;

  int nrows = val->nrows;
  int ncols = val->ncols;

  for (i = row_start; i < row_end; i++){
    for (j = 0; j < ncols; j++){
      current_state = check_status(BOARD(inboard, i, j));
      neighbor_count = neighbor_count(inboard, i, j);
      

	  if (!current_state) {
		  if (neighbor_count == 3) {
			  inorth = mod(i - 1, nrows);
			  isouth = mod(i + 1, nrows);
			  jwest = mod(j - 1, ncols);
			  jeast = mod(j + 1, ncols);

			  dead_to_alive(BOARD(outboard, i, j));
			  INCRE_COUNT(outboard, inorth, jwest);
			  INCRE_COUNT(outboard, inorth, j);
			  INCRE_COUNT(outboard, inorth, jeast);
			  INCRE_COUNT(outboard, i, jwest);
			  INCRE_COUNT(outboard, i, jeast);
			  INCRE_COUNT(outboard, isouth, jwest);
			  INCRE_COUNT(outboard, isouth, j);
			  INCRE_COUNT(outboard, isouth, jeast);
		  }
	  }
	  else if (current_state) {
		  if (neighbor_count <= 1 || neighbor_count >= 4) {
			  inorth = mod(i - 1, nrows);
			  isouth = mod(i + 1, nrows);
			  jwest = mod(j - 1, ncols);
			  jeast = mod(j + 1, ncols);

			  alive_to_dead(BOARD(outboard, i, j));
			  DECRE_COUNT(outboard, inorth, jwest);
			  DECRE_COUNT(outboard, inorth, j);
			  DECRE_COUNT(outboard, inorth, jeast);
			  DECRE_COUNT(outboard, i, jwest);
			  DECRE_COUNT(outboard, i, jeast);
			  DECRE_COUNT(outboard, isouth, jwest);
			  DECRE_COUNT(outboard, isouth, j);
			  DECRE_COUNT(outboard, isouth, jeast);
		  }

	  }
    }
  }

  pthread_exit(NULL);
}


void process_first_last(char* outboard, char* inboard, const int nrows, const int ncols, int row_start, int num_rows)
{
  int i, j;
  int current_state, neighbor_count, next_state;
  int inorth, isouth, jwest, jeast;
  int row_end = row_start + num_rows;

  for (i = row_start; i < row_end; i += (num_rows - 1)) {
    for (j = 0; j < ncols; j++){
      current_state = check_status(BOARD(inboard, i, j));
      neighbor_count = neighbor_count(inboard, i, j);

      //printf("i = %d, j = %d, north_west_count = %d\n", i, j, BOARD(count_board, inorth, jwest));

	  if (!current_state) {
		  if (neighbor_count == 3) {
			  inorth = mod(i - 1, nrows);
			  isouth = mod(i + 1, nrows);
			  jwest = mod(j - 1, ncols);
			  jeast = mod(j + 1, ncols);

			  dead_to_alive(BOARD(outboard, i, j));
			  INCRE_COUNT(outboard, inorth, jwest);
			  INCRE_COUNT(outboard, inorth, j);
			  INCRE_COUNT(outboard, inorth, jeast);
			  INCRE_COUNT(outboard, i, jwest);
			  INCRE_COUNT(outboard, i, jeast);
			  INCRE_COUNT(outboard, isouth, jwest);
			  INCRE_COUNT(outboard, isouth, j);
			  INCRE_COUNT(outboard, isouth, jeast);
		  }
	  }
	  else if (current_state) {
		  if (neighbor_count <= 1 || neighbor_count >= 4) {
			  inorth = mod(i - 1, nrows);
			  isouth = mod(i + 1, nrows);
			  jwest = mod(j - 1, ncols);
			  jeast = mod(j + 1, ncols);

			  alive_to_dead(BOARD(outboard, i, j));
			  DECRE_COUNT(outboard, inorth, jwest);
			  DECRE_COUNT(outboard, inorth, j);
			  DECRE_COUNT(outboard, inorth, jeast);
			  DECRE_COUNT(outboard, i, jwest);
			  DECRE_COUNT(outboard, i, jeast);
			  DECRE_COUNT(outboard, isouth, jwest);
			  DECRE_COUNT(outboard, isouth, j);
			  DECRE_COUNT(outboard, isouth, jeast);
		  }
	  }
	}
  }
}


void count_board_init(char* inboard, char* outboard, const int nrows, const int ncols) {

  int i,j;
  int inorth, isouth, jwest, jeast;

  
  for (i = 0; i < nrows; i++) {
	  for (j = 0; j < ncols; j++) {
		  if (BOARD(inboard, i, j) == 1) {
			  BOARD(inboard, i, j) = (char) 0x10;
			  BOARD(outboard, i, j) = (char) 0x10;
		  }
	  }
  }
  

  for (i = 0; i < nrows; i++) {
	  for (j = 0; j < ncols; j++) {
		  if (check_status(BOARD(inboard, i, j))) {
			  inorth = mod(i - 1, nrows);
			  isouth = mod(i + 1, nrows);
			  jwest = mod(j - 1, ncols);
			  jeast = mod(j + 1, ncols);

			  INCRE_COUNT(outboard, inorth, jwest);
			  INCRE_COUNT(outboard, inorth, j);
			  INCRE_COUNT(outboard, inorth, jeast);
			  INCRE_COUNT(outboard, i, jwest);
			  INCRE_COUNT(outboard, i, jeast);
			  INCRE_COUNT(outboard, isouth, jwest);
			  INCRE_COUNT(outboard, isouth, j);
			  INCRE_COUNT(outboard, isouth, jeast);
		  }
	  }
  }
}


void print_board(char* board, const int nrows, const int ncols, char* message) {
  int i, j;
  printf("%s\n", message);
  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      printf("%d ", BOARD(board, i, j));
    }
    printf("\n");
  }
}
