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

typedef struct {
    int *literals;          // 原始文字
    int *assignment_status; // 是否被赋值
    int original_length;
    int current_length;
    int is_satisfied;
    int unit_literal;       // 单子句文字缓存
} Clause;

/*-------------------- 文字出现记录 --------------------*/
typedef struct {
    int clause_index;
    int literal_position;
} LiteralOccurrence;

typedef struct {
    LiteralOccurrence *list;
    int count;
    int capacity;
} LiteralOccurrenceList;
typedef struct { int clause_index, literal_position; } ChangeRecord;
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

/* 扁平结构替代 literal_info */
extern LiteralOccurrenceList *pos_literals, *neg_literals;
extern uint8_t *in_unit_pos, *in_unit_neg; // 标记是否在单位子句堆中


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
bool findUnassignedLocation(int grid[SIZE][SIZE], int *row, int *col) ;
bool solveSudoku(int grid[SIZE][SIZE]) ;
bool createFullGridLasVegas(int grid[SIZE][SIZE], int preFilled) ;
void createFullGrid(int grid[SIZE][SIZE]) ;
void digHoles(int grid[SIZE][SIZE], int holes) ;
//satsolver.cpp
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


//cnftosudoku.cpp
int CnftoSudoku(const char* filename);
void printSolvedGrid();

//sudotocnf.cpp
void writeSudokuToCNF(int grid[SIZE][SIZE], const char* filename);
int ChangetoLiteral(int row, int col, int num);


#endif