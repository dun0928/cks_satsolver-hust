#include "definition.h"
#include "soudu.hpp"
#include "satsolver.hpp"
#include "sudutocnf.hpp"
#include "cnftosudoku.hpp"

int method=0;


int main(){

    int choice;
    printf("��ѡ�����:\n1. ������������\n2. ��ȡCNF�ļ����������\n0. �˳�\n");
    scanf("%d", &choice);
    while(choice==1||choice==2){
        if(choice==1){
            srand(time(0)); // ��ʼ�����������
            createFullGrid(grid);
            printf("������������:\n");
            printGrid(grid);
            digHoles(grid, 40); 
            printf("�ڶ������������:\n");
            printGrid(grid);
            int next;
            printf("��ѡ�����:\n1. sukudo����\n2. չʾsukudo���\n0. �˳�����һ��\n ");
            scanf("%d", &next);
            if(next!=0&&next!=1&&next!=2){
                printf("�����������������\n");
            }
            while(next==1||next==2){
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
                }
                printf("��ѡ�����:\n1. sukudo����\n2. չʾsukudo���\n0. �˳�\n ");
                scanf("%d", &next);
                if(next!=0&&next!=1&&next!=2){
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
            time1=satsolver(filename,method);
            method=2;
            time2=satsolver(filename,method);
            //printf("111");
            double optimization;
            optimization=(double)(time2-time1)/time2;
            printf("�Ż���Ϊ%.2f%\n",optimization*100);
        }
        printf("��ѡ�����:\n1. ������������\n2. ��ȡCNF�ļ����������\n0. �˳�\n");
        scanf("%d", &choice);

    }

}