#include "definition.h"
#include "soudu.hpp"
#include "satsolver.hpp"
#include "sudutocnf.hpp"
#include "cnftosudoku.hpp"

int main(){

    int choice;
    printf("请选择操作:\n1. 生成数独谜题\n2. 读取CNF文件并求解数独\n0. 退出\n");
    scanf("%d", &choice);
    while(choice==1||choice==2){
        if(choice==1){
            srand(time(0)); // 初始化随机数种子
            createFullGrid(grid);
            printf("完整数独网格:\n");
            printGrid(grid);
            digHoles(grid, 40); 
            printf("挖洞后的数独谜题:\n");
            printGrid(grid);
            int next;
            printf("请选择操作:\n1. sukudo处理\n2. 展示sukudo结果\n0. 退出到上一级\n ");
            scanf("%d", &next);
            if(next!=0&&next!=1&&next!=2){
                printf("输入错误，请重新输入\n");
            }
            while(next==1||next==2){
                if(next==1){   
                    writeSudokuToCNF(grid, "sudoku.cnf");
                    char sukodufile[]="sudoku.cnf";
                    //printf("111");
                    satsolver(sukodufile);
                    //printf("222");
                    CnftoSudoku("sudoku.res");
                    //printf("333");

                }else if(next==2){
                    printf("数独结果:\n");
                    printSolvedGrid();
                }else if(next==0){
                    break;
                }
                printf("请选择操作:\n1. sukudo处理\n2. 展示sukudo结果\n0. 退出\n ");
                scanf("%d", &next);
                if(next!=0&&next!=1&&next!=2){
                    printf("输入错误，请重新输入\n");
                }
            }
        }
        else if(choice==2){
            char filename[256];
            printf("请输入CNF文件名: ");
            scanf("%s", filename);
            satsolver(filename);
        }
        printf("请选择操作:\n1. 生成数独谜题\n2. 读取CNF文件并求解数独\n0. 退出\n");
        scanf("%d", &choice);

    }

}