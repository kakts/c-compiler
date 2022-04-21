#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid argments. please pass value. eg. ./a.out 42 ");
        return 1;
    }

    // 出力するアセンブリコードを表示
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("  mov rax, %d\n", atoi(argv[1])); // set return value to rax register.
    printf("  ret\n");
    return 0;
}