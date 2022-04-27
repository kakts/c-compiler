#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークン種類
typedef enum {
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;


typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind; // トークンの種類
    Token *next;    // 次の入力トークン
    int val;        // kindがTK＿NUMの場合、その数値
    char *str;      // トークン文字列
};

/**
 * トークン列
 * 
 * パーサは連結リストになっているトークンを読み進める。
 * このコードでおこなっているように、入力トークン列をstdinのようなストリームとして扱う方がコードが読みやすくなることが多いため
 * このスタイルにしている。
 * 
 * tokenを直接触るコードはconsumeやexpectといった関数に分け、それ以外の関数では直接触らないようにしている
 */
// 現在着目しているトークン
Token *token;

// エラーを報告するための関数
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号のときは、トークンを1つ読み進めtrueを返す
// それ以外の場合にはfalse
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときは、トークンを1つ読み進め
// それ以外の場合にはエラーを報告する
void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        error("'%c'ではありません", op);
    }
    token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す
// それ以外の場合にはエラー
int expect_number() {
    if (token->kind != TK_NUM) {
        error("not a number");
    }

    int val = token->val;
    token = token->next;
    return val;
}

// EOFに達しているか判定する
bool at_eof() {
    return token->kind == TK_EOF;
}

/**
 * 新しいトークンを作成してcurにつなげる
 * 
 * メモリ割り当てにcallocを使用している
 * mallocと異なり、callocは割り当てられたメモリをゼロクリアする
 * 要素をゼロクリアする手間を省くためにcallocを使用する
 */
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

/**
 * 入力文字列pをトークナイズしてそれを返す
 * 
 * 連結リストを構築する。
 * ダミーのhead要素を作り、そこに新しい要素をつなげていき、最後にhead->nextを返すようにするとコードが簡単になる
 * この方法では、headに割り当てられたメモリアほとんど無駄になるが、ローカル変数をアロケートするコストはほぼゼロのため
 * 気にしなくて良い
 */
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("トークナイズできない");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid argments. please pass value. eg. ./a.out 42 ");
        return 1;
    }

    token = tokenize(argv[1]);

    // 出力するアセンブリコードを表示
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // 式の最初は数でなければならないので、それをチェックし、最初のmov命令を出力
    printf("  mov rax, %d\n", expect_number());
    /**
     * `+ <数>` あるいは `- <数>` というトークンの並びを消費しつつアセンブリを出力
     */
    while (!at_eof()) {
        if (consume('+')) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub rax, %d\n", expect_number());
    }

    printf("  ret\n");
    return 0;

}