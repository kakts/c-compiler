#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid argments. please pass value. eg. ./a.out 42 ");
        return 1;
    }

    char *p = argv[1];

    // 出力するアセンブリコードを表示
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("  mov rax, %ld\n", strtol(p, &p, 10)); // set return value to rax register.

    // 数式の解析
    while (*p) {
        if (*p == '+') {
            p++;
            printf("  add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        if (*p == '-') {
            p++;
            printf("  sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "予期しない文字です: '%c'\n", *p);
        return 1;
    }

    printf("  ret\n");
    return 0;

}