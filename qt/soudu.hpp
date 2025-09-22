#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include<time.h>
#include "definition.h"


// �ļ���soudu.h
// �������к�������֮ǰ������ #include ����

#ifdef USE_QT_GUI
    #include "qt/sudoku_gui.hpp"
    #define interactiveMode() runInteractiveGui(entireGrid, playerGrid, initplaygrid)
#endif


#define SIZE 9
#define BOX_SIZE 3

int grid[SIZE][SIZE]; // ������������
int playerGrid[SIZE][SIZE]; // ��������
int entireGrid[SIZE][SIZE]; // ������������
int initplaygrid[SIZE][SIZE]; // �ڶ����ʼ���

// ��ӡ��������
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

// ��������������Ƿ���Ч
bool isSafeInRow(int grid[SIZE][SIZE], int row, int num) {
    for (int col = 0; col < SIZE; col++) {
        if (grid[row][col] == num) {
            return false;
        }
    }
    return true;
}

// ��������������Ƿ���Ч
bool isSafeInCol(int grid[SIZE][SIZE], int col, int num) {
    for (int row = 0; row < SIZE; row++) {
        if (grid[row][col] == num) {
            return false;
        }
    }
    return true;
}

// ���������3x3�������Ƿ���Ч
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

// ��������ڷ��Խ������Ƿ���Ч
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

// ������ַ����Ƿ�ȫ
bool isSafe(int grid[SIZE][SIZE], int row, int col, int num) {
    return isSafeInRow(grid, row, num) &&
           isSafeInCol(grid, col, num) &&
           isSafeInBox(grid, row - row % BOX_SIZE, col - col % BOX_SIZE, num) &&
           isSafeInDiagonal(grid, row, col, num) &&
           isSafeInWindow(grid, row, col, num);
}

// ����δ�����λ��
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

// �������
bool solveSudoku(int grid[SIZE][SIZE]) {
    int row, col;
    
    if (!findholes(grid, &row, &col)) {
        return true; // ����λ�ö�������
    }
    
    int numbers[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 9; i++) {
        int j = rand() % 9;
        int temp = numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = temp;
    }
    
    // ����������е�����
    for (int i = 0; i < 9; i++) {
        int num = numbers[i];
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;
            
            if (solveSudoku(grid)) {
                return true;
            }
            
            grid[row][col] = 0; // ����
        }
    }
    
    return false; 
}

// ʹ����˹ά��˹�㷨������������������
bool createFullGridLasVegas(int grid[SIZE][SIZE], int preFilled) {

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = 0;
        }
    }
    
    // �����䵥Ԫ��
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
    
    // ���Խ��ʣ�µĲ���
    return solveSudoku(grid);
}


void createFullGrid(int grid[SIZE][SIZE]) {
    int preFilled = 12;
    clock_t start_time, current_time;
    double elapsed = 0.0;
    const double TIME_LIMIT = 1.0; // 1�볬ʱ

    start_time = clock();

    while (!createFullGridLasVegas(grid, preFilled)) {
        current_time = clock();
        elapsed = ((double)(current_time - start_time)) / CLOCKS_PER_SEC;

        if (elapsed > TIME_LIMIT) {
            // ��ʱ�����µ�������
            createFullGrid(grid);
            return; // ��Ҫ������ݹ鷵�غ����ִ��
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
                grid[row][col] = backup; // ����
            }
        }
    }

    //printf("�ɹ��ڵ� %d ����������Ψһ�⡣\n", count);
}

// ���������������
int countSolutions(int grid[SIZE][SIZE]) {
    int row, col;

    if (!findholes(grid, &row, &col)) {
        return 1; // ��������
    }

    int count = 0;

    for (int num = 1; num <= 9; num++) {
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;

            count += countSolutions(grid);

            grid[row][col] = 0; // ����

            if (count > 1) {
                return count; 
            }
        }
    }

    return count;
}

// ����Ƿ���Ψһ��
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

//��������ڱ༭�ľ����ӡ����
void printPlayerGrid()
{
    printf("\n��ǰ���棨���д� 0 ��ʼ�ƣ���\n");
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
// ������������1 ������2 ɾ����0 �˳� 
void interactiveMode()
{
    int op, r, c, v, cnt;
    memcpy(playerGrid, grid, sizeof(grid));
    while (1) {
        printPlayerGrid();
        printf("\n��ѡ�������1 ����  2 ɾ��  0 �˳�\n");
        if (scanf("%d", &op) != 1) continue;
        if (op == 0) break;

        if (op == 1) {                        
            printf("��ʽ���� ��  ����д���       ");
            printf("ʾ����0 0 5 1 2 3  ��ʾ (0,0)->5, (1,2)->3\n");
            printf("����-1����\n");
            while (scanf("%d", &r) == 1 && r != -1) {
                if (scanf("%d %d", &c, &v) != 2) break;
                if (r < 0 || r >= SIZE || c < 0 || c >= SIZE || v < 1 || v > 9) {
                    printf("��������ַǷ�������\n");
                    continue;
                }
                if (playerGrid[r][c] != 0) {
                    printf("(%d,%d) �������� %d�����踲������ɾ��\n", r, c, playerGrid[r][c]);
                    continue;
                }
               
                playerGrid[r][c] = v;
            }
        }
        else if (op == 2) {                  
            printf("һ�ο�ɾ�������ʽ���� �� [�� ��]...\n");
            printf("����-1��������ɾ��\n");
            while (scanf("%d", &r) == 1 && r != -1) {
                if (scanf("%d", &c) != 1) break;
                if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) {
                    printf("����Ƿ�������\n");
                    continue;
                }
                playerGrid[r][c] = 0;
            }
        }
        else {
            printf("����Ƿ���������ѡ��\n");
        }
    }
    printf("�������������վ����ѱ����� playerGrid �С�\n");
}
#endif

//����
// int main() {
//     srand(time(0)); // ��ʼ�����������
    
//     int grid[SIZE][SIZE];
    
//     printf("���������İٷֺ�����...\n");
//     createFullGrid(grid);
    
//     printf("��������:\n");
//     printGrid(grid);
    
//     printf("\n�ڶ������ɵ���������:\n");
//     digHoles(grid, 40); // ��40����
//     printGrid(grid);
    
//     return 0;
// }