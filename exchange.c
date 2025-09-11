#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "用法: %s <文件名>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *input = fopen(filename, "r");
    if (!input) {
        perror("无法打开文件");
        return 1;
    }

    // 创建临时文件
    FILE *temp = fopen("temp_no_newline.txt", "w");
    if (!temp) {
        perror("无法创建临时文件");
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

    // 用临时文件替换原文件
    if (remove(filename) != 0) {
        perror("删除原文件失败");
        return 1;
    }

    if (rename("temp_no_newline.txt", filename) != 0) {
        perror("重命名临时文件失败");
        return 1;
    }

    printf("已删除文件中的所有换行符。\n");
    return 0;
}