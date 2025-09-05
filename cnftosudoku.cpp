// 这个文件的目的是把CNF格式的数独解读出来，转换为数独二维数组
#include "defination.h"
#include "soudu.cpp"
#include <cstring> // 添加string.h头文件

int solvedGrid[SIZE][SIZE] = {0}; // 初始化为0

int CnftoSudoku(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("无法打开文件: %s\n", filename);
        return 0;
    }

    char line[10000];
    while (fgets(line, sizeof(line), file)) {
        // 跳过注释行和无解情况
        if (line[0] == 's') {
            if (strstr(line, "UNSAT") != NULL) {
                printf("无解\n");
                fclose(file);
                return 0;
            }
            continue;
        }
        if (line[0] == 'c' || line[0] == 't') continue;
        
        char* token = strtok(line, " ");
        // 跳过行首的'v'
        if (token != NULL && strcmp(token, "v") == 0) {
            token = strtok(NULL, " ");
        }
        
        while (token != NULL) {
            int literal = atoi(token);
            if (literal == 0) break; // 行结束
            
            if (literal > 0) {
                // CNF变量到数独网格的转换
                int var_index = literal - 1; // 转换为0-based索引
                int row = var_index / (9 * 9);
                int col = (var_index % (9 * 9)) / 9;
                int num = (var_index % 9) + 1;
                
                if (row >= 0 && row < SIZE && col >= 0 && col < SIZE) {
                    solvedGrid[row][col] = num;
                }
            }
            
            token = strtok(NULL, " ");
        }
    }
    
    fclose(file);
    return 1;
}

void printSolvedGrid() {
    printf("数独解:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", solvedGrid[i][j]);
        }
        printf("\n");
    }
}

int main() {
    char filename[256];
    printf("请输入CNF文件名: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        printf("读取输入失败\n");
        return 1;
    }
    filename[strcspn(filename, "\n")] = 0;  // 去除换行符
    
    // 初始化网格
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            solvedGrid[i][j] = 0;
        }
    }
    
    if (CnftoSudoku(filename)) {
        printSolvedGrid();
    } else {
        printf("转换失败或无解\n");
    }
    return 0;
}