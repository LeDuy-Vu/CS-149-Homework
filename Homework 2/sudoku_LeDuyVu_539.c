#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define GRID_SIZE 9
#define SUBGRID_SIZE 3
#define THREADS_NUM 27

typedef struct
{
	int *data ; //contains array of 9 value
	int index ; //index of row or column or grid
} parameter ;

void printPuzzle(int*) ;
parameter *getRow(int*, int) ;
parameter *getCol(int*, int) ;
parameter *getGrid(int*, int) ;
void *checkRow(void*) ;
void *checkCol(void*) ;
void *checkGrid(void*) ;

int flags[THREADS_NUM] ; //global flags to check validity of puzzle

int main(void)
{
	puts("CS149 Sudoku from Le Duy Vu") ;
	int valid ; //validity status of the whole puzzle. 1 = valid and vice versa
	
	//Total of 27 threads to check each row, each column, and each grid
	pthread_t rowChecker[GRID_SIZE] ;
	pthread_t colChecker[GRID_SIZE] ;
	pthread_t gridChecker[GRID_SIZE] ;

	int puzzle1[GRID_SIZE][GRID_SIZE] = {
		{7, 3, 9, 8 ,4, 6, 1, 2, 5},
		{4, 1, 2, 9, 7, 5, 8, 6, 3},
		{8, 6, 5, 2, 3, 1, 9, 4, 7},
		{5, 4, 7, 6, 0, 3, 2, 1, 8},
		{3, 9, 1, 7, 8, 2, 4, 5, 6},
		{2, 8, 6, 5, 1, 4, 3, 7, 9},
		{9, 2, 8, 4, 6, 7, 5, 3, 1},
		{1, 7, 4, 3, 5, 9, 1, 8, 2},
		{6, 5, 3, 1, 2, 8, 7, 9, 4}} ;

	int puzzle2[GRID_SIZE][GRID_SIZE] = {
		{4, 1, 9, 3, 7, 6, 5, 8, 2},
		{3, 6, 7, 8, 5, 2, 1, 4, 9},
		{2, 8, 5, 1, 9, 4, 3, 7, 6},
		{6, 4, 1, 9, 3, 7, 8, 2, 5},
		{9, 3, 2, 4, 8, 5, 6, 1, 7},
		{5, 7, 8, 2, 6, 1, 4, 9, 3},
		{8, 5, 6, 7, 4, 9, 2, 3, 1},
		{1, 9, 4, 5, 2, 3, 7, 6, 8},
		{7, 2, 3, 6, 1, 8, 9, 5, 4}} ;

	//2-time loop for puzzle 1 and 2
	for (int i = 0; i < 2; i++)
	{
		valid = 1 ; //set initial validity to yes

		if (i == 0)
		{
			puts("\nPuzzle 1:\n") ;
			printPuzzle(*puzzle1) ;
			
			//Create 9 threads of each type
			for (int i = 0; i < GRID_SIZE; i++)
				//Check for failed thread creation
				if (pthread_create(&rowChecker[i], NULL, checkRow, getRow(*puzzle1, i)) != 0
				|| pthread_create(&colChecker[i], NULL, checkCol, getCol(*puzzle1, i)) != 0
				|| pthread_create(&gridChecker[i], NULL, checkGrid, getGrid(*puzzle1, i)) != 0)
				{
					printf("\nCreate thread failed\n") ;
					exit(1);
				}
		}
		else
		{
			puts("\nPuzzle 2:\n") ;
			printPuzzle(*puzzle2) ;
			
			//Create 9 threads of each type
			for (int i = 0; i < GRID_SIZE; i++)
				//Check for failed thread creation
				if (pthread_create(&rowChecker[i], NULL, checkRow, getRow(*puzzle2, i)) != 0
				|| pthread_create(&colChecker[i], NULL, checkCol, getCol(*puzzle2, i)) != 0
				|| pthread_create(&gridChecker[i], NULL, checkGrid, getGrid(*puzzle2, i)) != 0)
				{
					printf("\nCreate thread failed\n") ;
					exit(1);
				}
		}

		//Join all threads
		for (int i = 0; i < GRID_SIZE; i++)
		{
			//Check for failed thread join
			if (pthread_join(rowChecker[i], NULL) != 0
			|| pthread_join(colChecker[i], NULL) != 0
			|| pthread_join(gridChecker[i], NULL) != 0)
			{
				printf("\nJoin thread failed\n") ;
				exit(1);
			}
		}
		
		//Check final flag to receive validity
		for (int i = 0; i < THREADS_NUM; i++)
			if (flags[i] == 0)
				valid = 0 ;
		if (valid == 0)
			puts("\nConclusion: Puzzle is invalid") ;
		else
			puts("\nConclusion: Puzzle is valid") ;
	}

	return 0;
}

/*
 * Prints puzzle content in format.
 * @param puzzle pointer to the puzzle
 */
void printPuzzle(int *puzzle)
{
	for (int i = 0; i < GRID_SIZE; i++)
	{
		printf("\t") ;
		for (int j = 0; j < GRID_SIZE; j++)
			printf("%d ", *(puzzle + i * GRID_SIZE + j)) ;
		puts("") ;
	}
	puts("") ;
}

/*
 * Copies content of a specific row from the puzzle to parameter.
 * @param puzzle pointer to the puzzle
 * @param i index of the row
 * @return pointer to a parameter struct
 */
parameter *getRow(int *puzzle, int i)
{
	parameter *param = (parameter *) malloc(sizeof(parameter)) ;
	int *r = (int *) malloc(GRID_SIZE * sizeof(int)) ;
	for (int j = 0; j < GRID_SIZE; j++)
		*(r + j) = *(puzzle + i * GRID_SIZE + j) ;
	param->data = r ;
	param->index = i ;
	return param;
}

/**
 * Copies content of a specific column from the puzzle to parameter.
 * @param puzzle pointer to the puzzle
 * @param i index of the column
 * @return pointer to a parameter struct
 */
parameter *getCol(int *puzzle, int i)
{
	parameter *param = (parameter *) malloc(sizeof(parameter)) ;
	int *c = (int *) malloc(GRID_SIZE * sizeof(int)) ;
	for (int j = 0; j < GRID_SIZE; j++)
		*(c + j) = *(puzzle + j * GRID_SIZE + i) ;
	param->data = c ;
	param->index = i ;
	return param;
}

/**
 * Copies content of a specific subgrid from the puzzle to parameter.
 * @param puzzle pointer to the puzzle
 * @param i index of the grid
 * @return pointer to a parameter struct
 */
parameter *getGrid(int *puzzle, int i)
{
	parameter *param = (parameter *) malloc(sizeof(parameter)) ;
	int *g = (int *) malloc(GRID_SIZE * sizeof(int)) ;
	for (int j = 0; j < SUBGRID_SIZE; j++)
		for (int k = 0; k < SUBGRID_SIZE; k++)
		{
			int r = (i / SUBGRID_SIZE) * SUBGRID_SIZE + j ;
			int c = (i % SUBGRID_SIZE) * SUBGRID_SIZE + k ;
			*(g + j * SUBGRID_SIZE + k) = *(puzzle + r * GRID_SIZE + c) ;
		}
	param->data = g ;
	param->index = i ;
	return param;
}

/**
 * Check for any kind of error in a row of sudoku.
 * @param arg pointer to parameter struct
 */
void *checkRow(void *arg)
{
	parameter *param = (parameter *)arg ;
	int j, valid = 1 ;
	int missing[GRID_SIZE] = {0} ;

	for (int i = 0; i < GRID_SIZE; i++)
	{
		if (*(param->data + i) < 1 || *(param->data + i) > 9)
		{
			printf("puzzle[%d][%d] = %d, not between 1 and 9\n",
					param->index, i, *(param->data + i)) ;
			valid = 0 ;
		}

		for (j = i + 1; j < GRID_SIZE; j++)
			if (*(param->data + i) == *(param->data + j))
			{
				printf("puzzle[%d][%d] = puzzle[%d][%d] = %d, "
						"duplicated value\n", param->index, i,
						param->index, j, *(param->data + i)) ;
				valid = 0 ;
			}

		for (j = 0; j < GRID_SIZE; j++)
			if (*(param->data + i) == j + 1)
				missing[j] = 1 ;
	}

	for (j = 0; j < GRID_SIZE; j++)
		if (missing[j] == 0)
		{
			printf("row[%d], missing value %d\n", param->index, j + 1) ;
			valid = 0 ;
		}

	j = param->index ;
	free(param->data) ;
	free(param) ;

	flags[j] = valid ;
	pthread_exit(flags + j) ;
}

/**
 * Check for any kind of error in a column of sudoku.
 * @param arg pointer to parameter struct
 */
void *checkCol(void *arg)
{
	parameter *param = (parameter *)arg ;
	int j, valid = 1 ;
	int missing[GRID_SIZE] = {0} ;

	for (int i = 0; i < GRID_SIZE; i++)
	{
		if (*(param->data + i) < 1 || *(param->data + i) > 9)
		{
			printf("puzzle[%d][%d] = %d, not between 1 and 9\n",
					i, param->index, *(param->data + i)) ;
			valid = 0 ;
		}

		for (j = i + 1; j < GRID_SIZE; j++)
			if (*(param->data + i) == *(param->data + j))
			{
				printf("puzzle[%d][%d] = puzzle[%d][%d] = %d, "
						"duplicated value\n", i, param->index, j,
						param->index, *(param->data + i)) ;
				valid = 0 ;
			}

		for (j = 0; j < GRID_SIZE; j++)
			if (*(param->data + i) == j + 1)
				missing[j] = 1 ;
	}

	for (j = 0; j < GRID_SIZE; j++)
		if (missing[j] == 0)
		{
			printf("column[%d], missing value %d\n", param->index, j + 1) ;
			valid = 0 ;
		}

	j = param->index ;
	free(param->data) ;
	free(param) ;

	flags[GRID_SIZE + j] = valid ;
	pthread_exit(flags + GRID_SIZE + j) ;
}

/**
 * Check for any kind of error in a grid of sudoku.
 * @param arg pointer to parameter struct
 */
void *checkGrid(void *arg)
{
	parameter *param = (parameter *)arg ;
	int j, valid = 1 ;
	int missing[GRID_SIZE] = {0} ;

	for (int i = 0; i < GRID_SIZE; i++)
	{
		int r = (param->index / SUBGRID_SIZE) * SUBGRID_SIZE + i / 3 ;
		int c = (param->index % SUBGRID_SIZE) * SUBGRID_SIZE + i % 3 ;

		if (*(param->data + i) < 1 || *(param->data + i) > 9)
		{
			printf("puzzle[%d][%d] = %d, not between 1 and 9\n",
					r, c, *(param->data + i)) ;
			valid = 0 ;
		}

		for (j = i + 1; j < GRID_SIZE; j++)
			if (*(param->data + i) == *(param->data + j))
			{
				int r1 = (param->index / SUBGRID_SIZE) * SUBGRID_SIZE + j / 3 ;
				int c1 = (param->index % SUBGRID_SIZE) * SUBGRID_SIZE + j % 3 ;
				printf("puzzle[%d][%d] = puzzle[%d][%d] = %d, "
						"duplicated value\n", r, c, r1,
						c1, *(param->data + i)) ;
				valid = 0 ;
			}

		for (j = 0; j < GRID_SIZE; j++)
			if (*(param->data + i) == j + 1)
				missing[j] = 1 ;
	}

	for (j = 0; j < GRID_SIZE; j++)
		if (missing[j] == 0)
		{
			printf("grid[%d], missing value %d\n", param->index, j + 1) ;
			valid = 0 ;
		}

	j = param->index ;
	free(param->data) ;
	free(param) ;

	flags[2 * GRID_SIZE + j] = valid ;
	pthread_exit(flags + 2 * GRID_SIZE + j) ;
}
