#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include<time.h>
#include "definition.h"


// 文件：soudu.h
// 放在所有函数声明之前，比如 #include 下面

#ifdef USE_QT_GUI
    #include "qt/sudoku_gui.hpp"
    #define interactiveMode() runInteractiveGui(entireGrid, playerGrid, initplaygrid)
#endif


#define SIZE 9
#define BOX_SIZE 3

int grid[SIZE][SIZE]; // 定义数独网格
int playerGrid[SIZE][SIZE]; // 交互数独
int entireGrid[SIZE][SIZE]; // 完整数独网格
int initplaygrid[SIZE][SIZE]; // 挖洞后初始格局

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
bool isSafeInWindow(int grid[SIZE][SIZE], int row, int col, int num) {

    if (row >= 1 && row <= 3 && col >= 1 && col <= 3) {
        for (int i = 1; i <= 3; i++) {
            for (int j = 1; j <= 3; j++) {
                if (grid[i][j] == num) {
                    return false;
                }
            }
        }
    }
    

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
bool findholes(int grid[SIZE][SIZE], int *row, int *col) {
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
    
    if (!findholes(grid, &row, &col)) {
        return true; // 所有位置都已填满
    }
    
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
    
    return false; 
}

// 使用拉斯维加斯算法创建完整的数独网格
bool createFullGridLasVegas(int grid[SIZE][SIZE], int preFilled) {

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = 0;
        }
    }
    
    // 随机填充单元格
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


void createFullGrid(int grid[SIZE][SIZE]) {
    int preFilled = 12;
    clock_t start_time, current_time;
    double elapsed = 0.0;
    const double TIME_LIMIT = 1.0; // 1秒超时

    start_time = clock();

    while (!createFullGridLasVegas(grid, preFilled)) {
        current_time = clock();
        elapsed = ((double)(current_time - start_time)) / CLOCKS_PER_SEC;

        if (elapsed > TIME_LIMIT) {
            // 超时，重新调用自身
            createFullGrid(grid);
            return; // 重要：避免递归返回后继续执行
        }

        preFilled++;
        if (preFilled > 20) {
            preFilled = 12;
        }
    }
}

void digHoles(int grid[SIZE][SIZE], int holes) {
    int count = 0;
    while (count < holes) {
        int row = rand() % SIZE;
        int col = rand() % SIZE;

        if (grid[row][col] != 0) {
            int backup = grid[row][col];
            grid[row][col] = 0;

            if (hasonly(grid)) {
                count++;
            } else {
                grid[row][col] = backup; // 回溯
            }
        }
    }

    //printf("成功挖掉 %d 个洞，保持唯一解。\n", count);
}

// 计算数独解的数量
int countSolutions(int grid[SIZE][SIZE]) {
    int row, col;

    if (!findholes(grid, &row, &col)) {
        return 1; // 有完整解
    }

    int count = 0;

    for (int num = 1; num <= 9; num++) {
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;

            count += countSolutions(grid);

            grid[row][col] = 0; // 回溯

            if (count > 1) {
                return count; 
            }
        }
    }

    return count;
}

// 检查是否有唯一解
bool hasonly(int grid[SIZE][SIZE]) {
    int gridCopy[SIZE][SIZE];

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            gridCopy[i][j] = grid[i][j];
        }
    }

    int solutions = countSolutions(gridCopy);
    return solutions == 1;
}

//把玩家正在编辑的局面打印出来
void printPlayerGrid()
{
    printf("\n当前局面（行列从 0 开始计）：\n");
    for (int i = 0; i < SIZE; i++) {
        if (i % 3 == 0 && i != 0) printf("------+-------+------\n");
        for (int j = 0; j < SIZE; j++) {
            if (j % 3 == 0 && j != 0) printf("| ");
            printf(playerGrid[i][j] == 0 ? ". " : "%d ", playerGrid[i][j]);
        }
        printf("\n");
    }
}

#ifndef USE_QT_GUI
// 交互主函数：1 填数，2 删数，0 退出 
void interactiveMode()
{
    int op, r, c, v, cnt;
    memcpy(playerGrid, grid, sizeof(grid));
    while (1) {
        printPlayerGrid();
        printf("\n请选择操作：1 填数  2 删数  0 退出\n");
        if (scanf("%d", &op) != 1) continue;
        if (op == 0) break;

        if (op == 1) {                        
            printf("格式：行 列  可填写多个       ");
            printf("示例：0 0 5 1 2 3  表示 (0,0)->5, (1,2)->3\n");
            printf("输入-1结束\n");
            while (scanf("%d", &r) == 1 && r != -1) {
                if (scanf("%d %d", &c, &v) != 2) break;
                if (r < 0 || r >= SIZE || c < 0 || c >= SIZE || v < 1 || v > 9) {
                    printf("坐标或数字非法，跳过\n");
                    continue;
                }
                if (playerGrid[r][c] != 0) {
                    printf("(%d,%d) 已有数字 %d，如需覆盖请先删数\n", r, c, playerGrid[r][c]);
                    continue;
                }
               
                playerGrid[r][c] = v;
            }
        }
        else if (op == 2) {                  
            printf("一次可删多个，格式：行 列 [行 列]...\n");
            printf("输入-1结束本次删数\n");
            while (scanf("%d", &r) == 1 && r != -1) {
                if (scanf("%d", &c) != 1) break;
                if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) {
                    printf("坐标非法，跳过\n");
                    continue;
                }
                playerGrid[r][c] = 0;
            }
        }
        else {
            printf("输入非法，请重新选择\n");
        }
    }
    printf("交互结束，最终局面已保存在 playerGrid 中。\n");
}
#endif

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