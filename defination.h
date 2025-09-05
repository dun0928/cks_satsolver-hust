#ifndef DEFINITION_H

#define DEFINITION_H



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>



#define SIZE 9



extern int grid[SIZE][SIZE];
extern int solvedGrid[SIZE][SIZE];


void printGrid(int grid[SIZE][SIZE]) ;
bool isSafeInRow(int grid[SIZE][SIZE], int row, int num) ;
bool isSafeInCol(int grid[SIZE][SIZE], int col, int num) ;
bool isSafeInBox(int grid[SIZE][SIZE], int startRow, int startCol, int num) ;
bool isSafeInWindow(int grid[SIZE][SIZE], int row, int col, int num) ;
bool isSafeInDiagonal(int grid[SIZE][SIZE], int row, int col, int num);
bool isSafe(int grid[SIZE][SIZE], int row, int col, int num) ;
bool findUnassignedLocation(int grid[SIZE][SIZE], int *row, int *col) ;
bool solveSudoku(int grid[SIZE][SIZE]) ;
bool createFullGridLasVegas(int grid[SIZE][SIZE], int preFilled) ;
void createFullGrid(int grid[SIZE][SIZE]) ;
void digHoles(int grid[SIZE][SIZE], int holes) ;



#endif