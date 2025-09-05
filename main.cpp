#include "definition.h"
#include "soudu.hpp"
#include "satsolver.hpp"
#include "sudutocnf.hpp"
#include "cnftosudoku.hpp"

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
                    satsolver(sukodufile);
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
            satsolver(filename);
        }
        printf("��ѡ�����:\n1. ������������\n2. ��ȡCNF�ļ����������\n0. �˳�\n");
        scanf("%d", &choice);

    }

}