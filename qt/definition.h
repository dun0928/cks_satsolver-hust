#ifndef DEFINITION_H

#define DEFINITION_H



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>


#define SIZE 9
#define MAX_VARS 100000
#define MAX_CLAUSES 1000000

#define TRUE 1
#define FALSE 0
#define SAT 1
#define UNSAT 0
#define UNASSIGNED -1
#define ASSIGNED 1
#define SATISFIED 1
#define SHRUNK 0
#define POSITIVE 1
#define NEGATIVE 0



extern int grid[SIZE][SIZE];
extern int solvedGrid[SIZE][SIZE];
extern int playerGrid[SIZE][SIZE];
extern int entireGrid[SIZE][SIZE];  
extern int initplaygrid[SIZE][SIZE];

typedef struct {
    int *literals;          // 原始文字
    int *assignment_status; // 是否被赋值
    int original_length;
    int current_length;
    int is_satisfied;
    int unit_literal;       // 单子句文字，用于赋值
} Clause;

typedef struct {
    int clause_index;
    int literal_position;
} LiteralOccurrence;

typedef struct {
    LiteralOccurrence *list;
    int count;
    int capacity;
} LiteralOccurrenceList;
typedef struct { int clause_index, literal_position; } ChangeRecord;//第一个为子句编号，第二个为文字在子句中的位置
typedef struct { int value; } VariableResult;

extern int num_vars, original_formula_length, current_formula_length, max_clause_length;
extern Clause *clauses;

extern int unit_clause_stack[MAX_CLAUSES];
extern int unit_stack_size ;

extern ChangeRecord change_stack[MAX_CLAUSES * 10];
extern int change_stack_top ;
extern int change_counts[MAX_VARS * 2][2]; // [depth][SATISFIED/SHRUNK]
extern int depth ;

extern int contradiction_found ;
extern int conflicting_literal ;
extern int dpll_call_count ;


extern LiteralOccurrenceList *pos_literals, *neg_literals;
extern int *in_unit_pos, *in_unit_neg; // 标记是否在单位子句堆中


extern VariableResult results[MAX_VARS + 1];



//下面为函数声明
//soudu.cpp
void printGrid(int grid[SIZE][SIZE]) ;
bool isSafeInRow(int grid[SIZE][SIZE], int row, int num) ;
bool isSafeInCol(int grid[SIZE][SIZE], int col, int num) ;
bool isSafeInBox(int grid[SIZE][SIZE], int startRow, int startCol, int num) ;
bool isSafeInWindow(int grid[SIZE][SIZE], int row, int col, int num) ;
bool isSafeInDiagonal(int grid[SIZE][SIZE], int row, int col, int num);
bool isSafe(int grid[SIZE][SIZE], int row, int col, int num) ;
bool findholes(int grid[SIZE][SIZE], int *row, int *col) ;
bool solveSudoku(int grid[SIZE][SIZE]) ;
bool createFullGridLasVegas(int grid[SIZE][SIZE], int preFilled) ;
void createFullGrid(int grid[SIZE][SIZE]) ;
void digHoles(int grid[SIZE][SIZE], int holes) ;
bool hasonly(int grid[SIZE][SIZE]) ;
int countSolutions(int grid[SIZE][SIZE]) ;
void printPlayerGrid();
void interactiveMode();

//satsolver.cpp
void printf_res(char *filename);
void print_cnf(const char* filename);
double satsolver(char *filename,int method);
void preprocess();
void assign_value(int literal);
void unassign_value(int literal);
int dpll();
int get_min_clause_length();
void get_literal_weight(int var, int clause_len, unsigned int* pos_weight, unsigned int* neg_weight);
int select_branching_variable();
void write_result(int result_value, double time_used, char* filename);
void free_memory();
int read_cnf_file(char* filename);
//heap.hpp
void init_min_heap();
void heap_insert(int len, int idx);



//cnftosudoku.cpp
int CnftoSudoku(const char* filename);
void printSolvedGrid();

//sudotocnf.cpp
void writeSudokuToCNF(int grid[SIZE][SIZE], const char* filename);
int ChangetoLiteral(int row, int col, int num);


#endif