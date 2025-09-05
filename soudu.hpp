#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include<time.h>
#include "definition.h"

#define SIZE 9
#define BOX_SIZE 3

int grid[SIZE][SIZE]; // 定义数独网格

// 打印数独网格
void printGrid(int grid[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        if (i % 3 == 0 && i != 0) {
            printf("------+-------+------\n");
        }
        for (int j = 0; j < SIZE; j++) {
            if (j % 3 == 0 && j != 0) {
                printf("| ");
            }
            if (grid[i][j] == 0) {
                printf(". ");
            } else {
                printf("%d ", grid[i][j]);
            }
        }
        printf("\n");
    }
}

// 检查数字在行中是否有效
bool isSafeInRow(int grid[SIZE][SIZE], int row, int num) {
    for (int col = 0; col < SIZE; col++) {
        if (grid[row][col] == num) {
            return false;
        }
    }
    return true;
}

// 检查数字在列中是否有效
bool isSafeInCol(int grid[SIZE][SIZE], int col, int num) {
    for (int row = 0; row < SIZE; row++) {
        if (grid[row][col] == num) {
            return false;
        }
    }
    return true;
}

// 检查数字在3x3宫格中是否有效
bool isSafeInBox(int grid[SIZE][SIZE], int startRow, int startCol, int num) {
    for (int row = 0; row < BOX_SIZE; row++) {
        for (int col = 0; col < BOX_SIZE; col++) {
            if (grid[row + startRow][col + startCol] == num) {
                return false;
            }
        }
    }
    return true;
}

// 检查数字在反对角线中是否有效
bool isSafeInDiagonal(int grid[SIZE][SIZE], int row, int col, int num) {
    if ((row + col) == 8) {
        for (int i = 0; i < SIZE; i++) {
            if (grid[i][SIZE - 1 - i] == num) {
                return false;
            }
        }
    }
    return true;
}

// 检查数字在窗口中是否有效
bool isSafeInWindow(int grid[SIZE][SIZE], int row, int col, int num) {
    // 第一个窗口 (2-4行, 2-4列)
    if (row >= 1 && row <= 3 && col >= 1 && col <= 3) {
        for (int i = 1; i <= 3; i++) {
            for (int j = 1; j <= 3; j++) {
                if (grid[i][j] == num) {
                    return false;
                }
            }
        }
    }
    
    // 第二个窗口 (6-8行, 6-8列)
    if (row >= 5 && row <= 7 && col >= 5 && col <= 7) {
        for (int i = 5; i <= 7; i++) {
            for (int j = 5; j <= 7; j++) {
                if (grid[i][j] == num) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

// 检查数字放置是否安全
bool isSafe(int grid[SIZE][SIZE], int row, int col, int num) {
    return isSafeInRow(grid, row, num) &&
           isSafeInCol(grid, col, num) &&
           isSafeInBox(grid, row - row % BOX_SIZE, col - col % BOX_SIZE, num) &&
           isSafeInDiagonal(grid, row, col, num) &&
           isSafeInWindow(grid, row, col, num);
}

// 查找未分配的位置
bool findUnassignedLocation(int grid[SIZE][SIZE], int *row, int *col) {
    for (*row = 0; *row < SIZE; (*row)++) {
        for (*col = 0; *col < SIZE; (*col)++) {
            if (grid[*row][*col] == 0) {
                return true;
            }
        }
    }
    return false;
}

// 解决数独
bool solveSudoku(int grid[SIZE][SIZE]) {
    int row, col;
    
    if (!findUnassignedLocation(grid, &row, &col)) {
        return true; // 所有位置都已填满
    }
    
    // 创建数字1-9的随机排列
    int numbers[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 9; i++) {
        int j = rand() % 9;
        int temp = numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = temp;
    }
    
    // 尝试随机排列的数字
    for (int i = 0; i < 9; i++) {
        int num = numbers[i];
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;
            
            if (solveSudoku(grid)) {
                return true;
            }
            
            grid[row][col] = 0; // 回溯
        }
    }
    
    return false; // 触发回溯
}

// 使用拉斯维加斯算法创建完整的数独网格
bool createFullGridLasVegas(int grid[SIZE][SIZE], int preFilled) {
    // 初始化网格为0
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = 0;
        }
    }
    
    // 随机填充一些单元格
    int count = 0;
    while (count < preFilled) {
        int row = rand() % SIZE;
        int col = rand() % SIZE;
        if (grid[row][col] == 0) {
            int num = rand() % 9 + 1;
            if (isSafe(grid, row, col, num)) {
                grid[row][col] = num;
                count++;
            }
        }
    }
    
    // 尝试解决剩下的部分
    return solveSudoku(grid);
}

// 创建完整的数独网格
void createFullGrid(int grid[SIZE][SIZE]) {
    // 尝试使用拉斯维加斯算法生成数独
    // 如果失败，增加预填充的数量并重试
    int preFilled = 12; // 初始预填充数量
    
    clock_t start_time, end_time;
    double cpu_time_used;
    start_time = clock();

    while (!createFullGridLasVegas(grid, preFilled)) {
        preFilled++;
        if (preFilled > 30) {
            preFilled = 12; // 重置，避免无限循环
        }
    }
    end_time = clock();
    cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("生成完整数独用时: %.2f 秒\n", cpu_time_used);
}

// 挖洞创建谜题
void digHoles(int grid[SIZE][SIZE], int holes) {
    int count = 0;
    while (count < holes) {
        int row = rand() % SIZE;
        int col = rand() % SIZE;
        
        if (grid[row][col] != 0) {
            grid[row][col] = 0;
            count++;
        }
    }
}

//函数
// int main() {
//     srand(time(0)); // 初始化随机数种子
    
//     int grid[SIZE][SIZE];
    
//     printf("创建完整的百分号数独...\n");
//     createFullGrid(grid);
    
//     printf("完整数独:\n");
//     printGrid(grid);
    
//     printf("\n挖洞后生成的数独谜题:\n");
//     digHoles(grid, 40); // 挖40个洞
//     printGrid(grid);
    
//     return 0;
// }