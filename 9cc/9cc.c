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

/**
 * トークン型
 * トークンによって文字数が異なるので、lenで文字数を持たせる
 */
struct Token {
    TokenKind kind; // トークンの種類
    Token *next;    // 次の入力トークン
    int val;        // kindがTK＿NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
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

/**
 * 入力プログラム
 * 
 * エラー時にどこでエラーが起きているかを表示するためにプログラム全体を保持する
 */
char *user_input;

/**
 * エラー箇所を報告する
 */
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラーを報告するための関数
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/**
 * // 次のトークンが期待している記号のときは、トークンを1つ読み進めtrueを返す
 * // それ以外の場合にはfalse
 */
bool consume(char *op) {
    /**
     * 複数の文字からなる記号をトークナイズする場合、長いトークンから先にトークナイズする必要がある
     * e.g: 残りの文字列が>から始まる場合
     * まずstrncmp(p, ">=", 2)のように>=である可能性から先にチェックしないで、>から始まっている可能性をチェックすると
     * >=が >と=という2つのトークンとして誤ってトークナイズされる
     */
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときは、トークンを1つ読み進め
// それ以外の場合にはエラーを報告する
void expect(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len || 
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す
// それ以外の場合にはエラー
int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "not a number");
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

/**
 * startswith
 */
bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

/**
 * 入力文字列`user_input`をトークナイズしてそれを返す
 * 
 * 連結リストを構築する。
 * ダミーのhead要素を作り、そこに新しい要素をつなげていき、最後にhead->nextを返すようにするとコードが簡単になる
 * この方法では、headに割り当てられたメモリアほとんど無駄になるが、ローカル変数をアロケートするコストはほぼゼロのため
 * 気にしなくて良い
 */
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        // multi-letter puctuator
        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // single-letter punctuator
        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "トークナイズできない");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

/**
 * ASTのノードの種類
 */
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// ASTのノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs; // 左辺
    Node *rhs; // 右辺
    int val; // kindがND_NUMの場合のみ使う
};

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

/**
 * 新規ノード作成
 */
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/**
 * parser
 * 
 * expr = equalityという生成規則に則ってパースする
 */
Node *expr() {
    return equality();
}

/**
 * parser
 * 
 * equality = relational ("==" relational | "!=" relational)*という生成規則に則ってパースする
 */
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("==")) {
            node = new_binary(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_binary(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

/**
 * parser
 * 
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*という生成規則に則ってパースする
 */
Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<")) {
            node = new_binary(ND_LT, node, add());
        } else if (consume("<=")) {
            node = new_binary(ND_LE, node, add());
        } else if (consume(">")) {
            node = new_binary(ND_LT, add(), node);
        } else if (consume(">=")) {
            node = new_binary(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

/**
 * parser
 * 
 * add = mul ("+" mul | "-" mul)*という生成規則に則ってパースする
 */
Node *add() {
    Node *node =mul();

    for (;;) {
        if (consume("+")) {
            node = new_binary(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_binary(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}


/**
 * mul = unary ("*" unary | "/" unary)* という生成規則に則ってパースする
 */
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_binary(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_binary(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

/**
 * 単項演算子用の規則
 * unary = ("+" | "-")? | primary という生成規則に則ってパースする
 */
Node *unary() {
    if (consume("+")) {
        // +x を xに置き換える
        return unary();
    }
    if (consume("-")) {
        // -x を 0 - xに置き換える
        return new_binary(ND_SUB, new_node_num(0), unary());
    }
    return primary();
}

/**
 * primary = "(" expr ")" | numという生成規則に則ってパースする
 */
Node *primary() {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    // そうでない場合は数値のはず
    return new_node_num(expect_number());
}

void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
    }
    
    printf("  push rax\n");
}


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid argments. please pass value. eg. ./a.out 42 ");
        return 1;
    }

    // トークナイズしてパースする
    user_input = argv[1];
    token = tokenize(user_input);
    Node *node = expr();

    // 出力するアセンブリコードを表示
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // ASTを下りながらコード生成
    gen(node);

    // スタックトップに式全体の値が残っているはず
    // それをRAXにロードして関数からの返り血とする

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}