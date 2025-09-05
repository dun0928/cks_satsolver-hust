#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include<time.h>
#include "definition.h"

#define SIZE 9
#define BOX_SIZE 3

int grid[SIZE][SIZE]; // ������������

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

// ��������ڴ������Ƿ���Ч
bool isSafeInWindow(int grid[SIZE][SIZE], int row, int col, int num) {
    // ��һ������ (2-4��, 2-4��)
    if (row >= 1 && row <= 3 && col >= 1 && col <= 3) {
        for (int i = 1; i <= 3; i++) {
            for (int j = 1; j <= 3; j++) {
                if (grid[i][j] == num) {
                    return false;
                }
            }
        }
    }
    
    // �ڶ������� (6-8��, 6-8��)
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

// �������
bool solveSudoku(int grid[SIZE][SIZE]) {
    int row, col;
    
    if (!findUnassignedLocation(grid, &row, &col)) {
        return true; // ����λ�ö�������
    }
    
    // ��������1-9���������
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
    
    return false; // ��������
}

// ʹ����˹ά��˹�㷨������������������
bool createFullGridLasVegas(int grid[SIZE][SIZE], int preFilled) {
    // ��ʼ������Ϊ0
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = 0;
        }
    }
    
    // ������һЩ��Ԫ��
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

// ������������������
void createFullGrid(int grid[SIZE][SIZE]) {
    // ����ʹ����˹ά��˹�㷨��������
    // ���ʧ�ܣ�����Ԥ��������������
    int preFilled = 12; // ��ʼԤ�������
    
    clock_t start_time, end_time;
    double cpu_time_used;
    start_time = clock();

    while (!createFullGridLasVegas(grid, preFilled)) {
        preFilled++;
        if (preFilled > 30) {
            preFilled = 12; // ���ã���������ѭ��
        }
    }
    end_time = clock();
    cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("��������������ʱ: %.2f ��\n", cpu_time_used);
}

// �ڶ���������
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