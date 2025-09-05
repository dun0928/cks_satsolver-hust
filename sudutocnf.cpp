//����ļ���Ŀ���ǰ���������ת��ΪCNF��ʽ��������SAT������������������һ��int�Ķ�ά���飬û�и�ֵ�ĵط�Ϊ.
#include "defination.h"
#include "soudu.cpp"

int ChangetoLiteral(int row, int col, int num){
    return (row-1)*81 + (col-1)*9 + num;
}

// ����������ת��ΪCNF��ʽ��д���ļ�
void writeSudokuToCNF(int grid[SIZE][SIZE], const char* filename){  

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("�޷������ļ�: %s\n", filename);
        return;
    } 
    
    int literalCount = 0;
    int clauseCount = 0;
    long headerPos = ftell(file); // ��¼��ǰλ�ã�ͷ��������λ�ã�
    fprintf(file, "p cnf %d %d       \n", SIZE*SIZE*SIZE, clauseCount);
    //����Ϊ��Լ��
    for(int i=1;i<=SIZE;i++){
        for(int j=1;j<=SIZE;j++){
            for(int k=1;k<=SIZE;k++){
                fprintf(file, "%d ", ChangetoLiteral(i, j, k));

            }
            fprintf(file, "0\n");
            clauseCount++;
            for(int k1=1;k1<=SIZE;k1++){
                for(int k2=k1+1;k2<=SIZE;k2++){
                    fprintf(file, "-%d -%d 0\n", ChangetoLiteral(i, j, k1), ChangetoLiteral(i, j, k2));
                    clauseCount++;
                }
            }
        }
    }
    //����Ϊ��Լ��
    for(int i=1;i<=SIZE;i++){//��һ�п�ʼ
        for(int j=1;j<=SIZE;j++){//ĳһ�к���ĳһ������
            for(int k=1;k<=SIZE;k++){//һ����ͬһ���ֵĲ�ͬ����
                fprintf(file, "%d ", ChangetoLiteral(i, k, j));
            }
            fprintf(file, "0\n");
            clauseCount++;
        }
        // for(int k1=1;k1<=SIZE;k1++){
        //     for(int k2=k1+1;k2<=SIZE;k2++){//��ͬ��������
        //         for(int j=1;j<=SIZE;j++){//���ֲ�������ͬ
        //             fprintf(file, "-%d ", i*100 + k1*10 + j);
        //             fprintf(file, "-%d 0\n", i*100 + k2*10 + j);
        //             clauseCount++;
        //         }
        //     }
        // }//�Ѿ�ȷ����һ��9�����ֲ��ظ���

    }

    //����Ϊ��Լ��
    for(int j=1;j<=SIZE;j++){//��һ�п�ʼ
        for(int i=1;i<=SIZE;i++){//ĳһ�к���ĳһ������
            for(int k=1;k<=SIZE;k++){//һ����ͬһ���ֵĲ�ͬ����
                fprintf(file, "%d ", ChangetoLiteral(k, j, i));
            }
            fprintf(file, "0\n");
            clauseCount++;
        }
        // for(int k1=1;k1<=SIZE;k1++){
        //     for(int k2=k1+1;k2<=SIZE;k2++){//��ͬ��������
        //         for(int i=1;i<=SIZE;i++){//���ֲ�������ͬ
        //             fprintf(file, "-%d ", k1*100 + j*10 + i);
        //             fprintf(file, "-%d 0\n", k2*100 + j*10 + i);
        //             clauseCount++;
        //         }
        //     }
        // }//�Ѿ�ȷ����һ��9�����ֲ��ظ���

    }
    for(int boxRow = 0; boxRow < 3; boxRow++) {
        for(int boxCol = 0; boxCol < 3; boxCol++) {
            for(int k = 1; k <= SIZE; k++) {
                // ÿ����������������k
                for(int i = 1; i <= 3; i++) {
                    for(int j = 1; j <= 3; j++) {
                        int row = boxRow * 3 + i;
                        int col = boxCol * 3 + j;
                        fprintf(file, "%d ", ChangetoLiteral(row, col, k));
                    }
                }
                fprintf(file, "0\n");
                clauseCount++;
                
                // ÿ������������kֻ�ܳ���һ��
                for(int pos1 = 0; pos1 < 9; pos1++) {
                    for(int pos2 = pos1 + 1; pos2 < 9; pos2++) {
                        int row1 = boxRow * 3 + (pos1 / 3) + 1;
                        int col1 = boxCol * 3 + (pos1 % 3) + 1;
                        int row2 = boxRow * 3 + (pos2 / 3) + 1;
                        int col2 = boxCol * 3 + (pos2 % 3) + 1;
                        fprintf(file, "-%d -%d 0\n", ChangetoLiteral(row1, col1, k), ChangetoLiteral(row2, col2, k));
                        clauseCount++;
                    }
                }
            }
        }
    }

    // ���Խ���Լ����i + j = 10�ĶԽ��ߣ�
    for(int k = 1; k <= SIZE; k++) {
        // ���Խ��߱����������k
        for(int i = 1; i <= SIZE; i++) {
            fprintf(file, "%d ", ChangetoLiteral(i, SIZE - i + 1, k));
        }
        fprintf(file, "0\n");
        clauseCount++;

        // ���Խ���������kֻ�ܳ���һ��
        for(int i1 = 1; i1 <= SIZE; i1++) {
            for(int i2 = i1 + 1; i2 <= SIZE; i2++) {
                fprintf(file, "-%d -%d 0\n", ChangetoLiteral(i1, SIZE - i1 + 1, k), ChangetoLiteral(i2, SIZE - i2 + 1, k));
                clauseCount++;
            }
        }
    }

    // ��һ������Լ����2-4��, 2-4�У�
    for(int k = 1; k <= SIZE; k++) {
        // ��һ�����ڱ����������k
        for(int i = 2; i <= 4; i++) {
            for(int j = 2; j <= 4; j++) {
                fprintf(file, "%d ", ChangetoLiteral(i, j, k));
            }
        }
        fprintf(file, "0\n");
        clauseCount++;
        
        // ��һ������������kֻ�ܳ���һ��
        for(int pos1 = 0; pos1 < 9; pos1++) {
            for(int pos2 = pos1 + 1; pos2 < 9; pos2++) {
                int i1 = 2 + (pos1 / 3);
                int j1 = 2 + (pos1 % 3);
                int i2 = 2 + (pos2 / 3);
                int j2 = 2 + (pos2 % 3);
                fprintf(file, "-%d -%d 0\n", ChangetoLiteral(i1, j1, k), ChangetoLiteral(i2, j2, k));
                clauseCount++;
            }
        }
    }

    // �ڶ�������Լ����6-8��, 6-8�У�
    for(int k = 1; k <= SIZE; k++) {
        // �ڶ������ڱ����������k
        for(int i = 6; i <= 8; i++) {
            for(int j = 6; j <= 8; j++) {
                fprintf(file, "%d ", ChangetoLiteral(i, j, k));
            }
        }
        fprintf(file, "0\n");
        clauseCount++;
        
        // �ڶ�������������kֻ�ܳ���һ��
        for(int pos1 = 0; pos1 < 9; pos1++) {
            for(int pos2 = pos1 + 1; pos2 < 9; pos2++) {
                int i1 = 6 + (pos1 / 3);
                int j1 = 6 + (pos1 % 3);
                int i2 = 6 + (pos2 / 3);
                int j2 = 6 + (pos2 % 3);
                fprintf(file, "-%d -%d 0\n", ChangetoLiteral(i1, j1, k), ChangetoLiteral(i2, j2, k));
                clauseCount++;
            }
        }
    }

    // ��֪���ֵĵ�Ԫ�Ӿ�
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(grid[i][j] != 0) {
                int row = i + 1;
                int col = j + 1;
                int num = grid[i][j];
                fprintf(file, "%d 0\n", ChangetoLiteral(row, col, num));
                clauseCount++;
            }
        }
    }

    // �ص��ļ�ͷ�������Ӿ�����
    fseek(file, headerPos, SEEK_SET);
    fprintf(file, "p cnf %d %d\n", SIZE*SIZE*SIZE, clauseCount);
    fclose(file);
    printf("CNF�ļ������ɣ���%d���Ӿ�\n", clauseCount);
    

}

int main(){
    srand(time(0)); // ��ʼ�����������
    
    printf("���������İٷֺ�����...\n");
    createFullGrid(grid);
    
    printf("��������:\n");
    printGrid(grid);
    
    printf("\n�ڶ������ɵ���������:\n");
    digHoles(grid, 40); // ��40����

    printGrid(grid);

    writeSudokuToCNF(grid, "sudoku.cnf");
    return 0;
}