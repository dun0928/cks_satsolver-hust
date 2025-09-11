#include "definition.h"
#include "soudu.hpp"
#include "satsolver.hpp"
#include "sudutocnf.hpp"
#include "cnftosudoku.hpp"

int method=0;


int main(){

    int choice;
    printf("请选择操作:\n1. 生成数独谜题\n2. 读取CNF文件并求解CNF\n0. 退出\n");
    scanf("%d", &choice);
    while(choice==1||choice==2){
        if(choice==1){
            srand(time(0)); // 初始化随机数种子
            createFullGrid(grid);
            printf("完整数独网格:\n");
            printGrid(grid);
            memcpy(entireGrid, grid, sizeof(grid));
            // 挖洞，生成谜题
            digHoles(grid, 40); 
            printf("挖洞后的数独谜题:\n");
            printGrid(grid);
            memcpy(playerGrid, grid, sizeof(grid));
            memcpy(initplaygrid, grid, sizeof(grid));
            int next;
            printf("请选择操作:\n1. sukudo处理\n2. 展示sukudo结果\n 3. 交互模式\n0. 退出到上一级\n ");
            scanf("%d", &next);
            if(next!=0&&next!=1&&next!=2&&next!=3){
                printf("输入错误，请重新输入\n");
            }
            while(next==1||next==2||next==3){
                if(next==1){   
                    writeSudokuToCNF(grid, "sudoku.cnf");
                    char sukodufile[]="sudoku.cnf";
                    //printf("111");
                    method=1;
                    satsolver(sukodufile,method);
                    //printf("222");
                    CnftoSudoku("sudoku.res");
                    //printf("333");

                }else if(next==2){
                    printf("数独结果:\n");
                    printSolvedGrid();
                }else if(next==0){
                    break;
                }else if(next==3){
                    interactiveMode();
                }
                
                printf("请选择操作:\n1. sukudo处理\n2. 展示sukudo结果\n 3.交互模式\n0. 退出\n ");
                scanf("%d", &next);
                if(next!=0&&next!=1&&next!=2&&next!=3){
                    printf("输入错误，请重新输入\n");
                }
            }
        }
        else if(choice==2){
            char filename[256];
            printf("请输入CNF文件名: ");
            scanf("%s", filename);
            int time1=0;
            int time2=0;
            method=1;
            printf("是否展示读取的cnf结果？1.是 0.否\n");
            int show;
            scanf("%d",&show);
            if(show==1){
                print_cnf(filename);
            }
            time1=satsolver(filename,method);
            method=2;
            time2=satsolver(filename,method);
            char *dot = strrchr(filename, '.');
            if (dot) {
            strcpy(dot, ".res");
            } else {
                strcat(filename, ".res");
            }
            printf_res(filename);
            //printf("111");
            double optimization;
            optimization=(double)(time2-time1)/time2;
            printf("优化率为%.2f%\n",optimization*100);
        }
        printf("请选择操作:\n1. 生成数独谜题\n2. 读取CNF文件并求解数独\n0. 退出\n");
        scanf("%d", &choice);

    }

}