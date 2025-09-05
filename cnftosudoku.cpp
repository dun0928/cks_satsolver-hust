// ����ļ���Ŀ���ǰ�CNF��ʽ���������������ת��Ϊ������ά����
#include "defination.h"
#include "soudu.cpp"
#include <cstring> // ���string.hͷ�ļ�

int solvedGrid[SIZE][SIZE] = {0}; // ��ʼ��Ϊ0

int CnftoSudoku(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("�޷����ļ�: %s\n", filename);
        return 0;
    }

    char line[10000];
    while (fgets(line, sizeof(line), file)) {
        // ����ע���к��޽����
        if (line[0] == 's') {
            if (strstr(line, "UNSAT") != NULL) {
                printf("�޽�\n");
                fclose(file);
                return 0;
            }
            continue;
        }
        if (line[0] == 'c' || line[0] == 't') continue;
        
        char* token = strtok(line, " ");
        // �������׵�'v'
        if (token != NULL && strcmp(token, "v") == 0) {
            token = strtok(NULL, " ");
        }
        
        while (token != NULL) {
            int literal = atoi(token);
            if (literal == 0) break; // �н���
            
            if (literal > 0) {
                // CNF���������������ת��
                int var_index = literal - 1; // ת��Ϊ0-based����
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
    printf("������:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", solvedGrid[i][j]);
        }
        printf("\n");
    }
}

int main() {
    char filename[256];
    printf("������CNF�ļ���: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        printf("��ȡ����ʧ��\n");
        return 1;
    }
    filename[strcspn(filename, "\n")] = 0;  // ȥ�����з�
    
    // ��ʼ������
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            solvedGrid[i][j] = 0;
        }
    }
    
    if (CnftoSudoku(filename)) {
        printSolvedGrid();
    } else {
        printf("ת��ʧ�ܻ��޽�\n");
    }
    return 0;
}