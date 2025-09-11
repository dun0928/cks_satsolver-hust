#include "definition.h"
#include "soudu.hpp"
#include "satsolver.hpp"
#include "sudutocnf.hpp"
#include "cnftosudoku.hpp"

int method=0;


int main(){

    int choice;
    printf("��ѡ�����:\n1. ������������\n2. ��ȡCNF�ļ������CNF\n0. �˳�\n");
    scanf("%d", &choice);
    while(choice==1||choice==2){
        if(choice==1){
            srand(time(0)); // ��ʼ�����������
            createFullGrid(grid);
            printf("������������:\n");
            printGrid(grid);
            memcpy(entireGrid, grid, sizeof(grid));
            // �ڶ�����������
            digHoles(grid, 40); 
            printf("�ڶ������������:\n");
            printGrid(grid);
            memcpy(playerGrid, grid, sizeof(grid));
            memcpy(initplaygrid, grid, sizeof(grid));
            int next;
            printf("��ѡ�����:\n1. sukudo����\n2. չʾsukudo���\n 3. ����ģʽ\n0. �˳�����һ��\n ");
            scanf("%d", &next);
            if(next!=0&&next!=1&&next!=2&&next!=3){
                printf("�����������������\n");
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
                    printf("�������:\n");
                    printSolvedGrid();
                }else if(next==0){
                    break;
                }else if(next==3){
                    interactiveMode();
                }
                
                printf("��ѡ�����:\n1. sukudo����\n2. չʾsukudo���\n 3.����ģʽ\n0. �˳�\n ");
                scanf("%d", &next);
                if(next!=0&&next!=1&&next!=2&&next!=3){
                    printf("�����������������\n");
                }
            }
        }
        else if(choice==2){
            char filename[256];
            printf("������CNF�ļ���: ");
            scanf("%s", filename);
            int time1=0;
            int time2=0;
            method=1;
            printf("�Ƿ�չʾ��ȡ��cnf�����1.�� 0.��\n");
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
            printf("�Ż���Ϊ%.2f%\n",optimization*100);
        }
        printf("��ѡ�����:\n1. ������������\n2. ��ȡCNF�ļ����������\n0. �˳�\n");
        scanf("%d", &choice);

    }

}