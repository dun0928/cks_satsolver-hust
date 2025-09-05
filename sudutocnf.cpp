//这个文件的目的是把数独问题转化为CNF格式，方便用SAT求解器来解决，数独是一个int的二维数组，没有赋值的地方为.
#include "defination.h"
#include "soudu.cpp"

int ChangetoLiteral(int row, int col, int num){
    return (row-1)*81 + (col-1)*9 + num;
}

// 将数独问题转换为CNF格式并写入文件
void writeSudokuToCNF(int grid[SIZE][SIZE], const char* filename){  

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("无法创建文件: %s\n", filename);
        return;
    } 
    
    int literalCount = 0;
    int clauseCount = 0;
    long headerPos = ftell(file); // 记录当前位置（头部结束的位置）
    fprintf(file, "p cnf %d %d       \n", SIZE*SIZE*SIZE, clauseCount);
    //下面为格约束
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
    //下面为行约束
    for(int i=1;i<=SIZE;i++){//第一行开始
        for(int j=1;j<=SIZE;j++){//某一行含有某一个数字
            for(int k=1;k<=SIZE;k++){//一行下同一数字的不同列数
                fprintf(file, "%d ", ChangetoLiteral(i, k, j));
            }
            fprintf(file, "0\n");
            clauseCount++;
        }
        // for(int k1=1;k1<=SIZE;k1++){
        //     for(int k2=k1+1;k2<=SIZE;k2++){//不同的两个格
        //         for(int j=1;j<=SIZE;j++){//数字不可以相同
        //             fprintf(file, "-%d ", i*100 + k1*10 + j);
        //             fprintf(file, "-%d 0\n", i*100 + k2*10 + j);
        //             clauseCount++;
        //         }
        //     }
        // }//已经确定这一行9个数字不重复了

    }

    //下面为列约束
    for(int j=1;j<=SIZE;j++){//第一列开始
        for(int i=1;i<=SIZE;i++){//某一列含有某一个数字
            for(int k=1;k<=SIZE;k++){//一列下同一数字的不同列数
                fprintf(file, "%d ", ChangetoLiteral(k, j, i));
            }
            fprintf(file, "0\n");
            clauseCount++;
        }
        // for(int k1=1;k1<=SIZE;k1++){
        //     for(int k2=k1+1;k2<=SIZE;k2++){//不同的两个格
        //         for(int i=1;i<=SIZE;i++){//数字不可以相同
        //             fprintf(file, "-%d ", k1*100 + j*10 + i);
        //             fprintf(file, "-%d 0\n", k2*100 + j*10 + i);
        //             clauseCount++;
        //         }
        //     }
        // }//已经确定这一列9个数字不重复了

    }
    for(int boxRow = 0; boxRow < 3; boxRow++) {
        for(int boxCol = 0; boxCol < 3; boxCol++) {
            for(int k = 1; k <= SIZE; k++) {
                // 每个宫格必须包含数字k
                for(int i = 1; i <= 3; i++) {
                    for(int j = 1; j <= 3; j++) {
                        int row = boxRow * 3 + i;
                        int col = boxCol * 3 + j;
                        fprintf(file, "%d ", ChangetoLiteral(row, col, k));
                    }
                }
                fprintf(file, "0\n");
                clauseCount++;
                
                // 每个宫格中数字k只能出现一次
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

    // 反对角线约束（i + j = 10的对角线）
    for(int k = 1; k <= SIZE; k++) {
        // 反对角线必须包含数字k
        for(int i = 1; i <= SIZE; i++) {
            fprintf(file, "%d ", ChangetoLiteral(i, SIZE - i + 1, k));
        }
        fprintf(file, "0\n");
        clauseCount++;

        // 反对角线中数字k只能出现一次
        for(int i1 = 1; i1 <= SIZE; i1++) {
            for(int i2 = i1 + 1; i2 <= SIZE; i2++) {
                fprintf(file, "-%d -%d 0\n", ChangetoLiteral(i1, SIZE - i1 + 1, k), ChangetoLiteral(i2, SIZE - i2 + 1, k));
                clauseCount++;
            }
        }
    }

    // 第一个窗口约束（2-4行, 2-4列）
    for(int k = 1; k <= SIZE; k++) {
        // 第一个窗口必须包含数字k
        for(int i = 2; i <= 4; i++) {
            for(int j = 2; j <= 4; j++) {
                fprintf(file, "%d ", ChangetoLiteral(i, j, k));
            }
        }
        fprintf(file, "0\n");
        clauseCount++;
        
        // 第一个窗口中数字k只能出现一次
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

    // 第二个窗口约束（6-8行, 6-8列）
    for(int k = 1; k <= SIZE; k++) {
        // 第二个窗口必须包含数字k
        for(int i = 6; i <= 8; i++) {
            for(int j = 6; j <= 8; j++) {
                fprintf(file, "%d ", ChangetoLiteral(i, j, k));
            }
        }
        fprintf(file, "0\n");
        clauseCount++;
        
        // 第二个窗口中数字k只能出现一次
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

    // 已知数字的单元子句
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

    // 回到文件头部更新子句数量
    fseek(file, headerPos, SEEK_SET);
    fprintf(file, "p cnf %d %d\n", SIZE*SIZE*SIZE, clauseCount);
    fclose(file);
    printf("CNF文件已生成，共%d个子句\n", clauseCount);
    

}

int main(){
    srand(time(0)); // 初始化随机数种子
    
    printf("创建完整的百分号数独...\n");
    createFullGrid(grid);
    
    printf("完整数独:\n");
    printGrid(grid);
    
    printf("\n挖洞后生成的数独谜题:\n");
    digHoles(grid, 40); // 挖40个洞

    printGrid(grid);

    writeSudokuToCNF(grid, "sudoku.cnf");
    return 0;
}