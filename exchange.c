#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "�÷�: %s <�ļ���>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *input = fopen(filename, "r");
    if (!input) {
        perror("�޷����ļ�");
        return 1;
    }

    // ������ʱ�ļ�
    FILE *temp = fopen("temp_no_newline.txt", "w");
    if (!temp) {
        perror("�޷�������ʱ�ļ�");
        fclose(input);
        return 1;
    }

    int ch;
    while ((ch = fgetc(input)) != EOF) {
        if (ch != '\n') {
            fputc(ch, temp);
        }
    }

    fclose(input);
    fclose(temp);

    // ����ʱ�ļ��滻ԭ�ļ�
    if (remove(filename) != 0) {
        perror("ɾ��ԭ�ļ�ʧ��");
        return 1;
    }

    if (rename("temp_no_newline.txt", filename) != 0) {
        perror("��������ʱ�ļ�ʧ��");
        return 1;
    }

    printf("��ɾ���ļ��е����л��з���\n");
    return 0;
}