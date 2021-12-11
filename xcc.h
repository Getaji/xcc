#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

// -----------------------------------------------------------------------------
// Parser
// -----------------------------------------------------------------------------

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_RETURN,   // return
  TK_IF,       // if
  TK_ELSE,     // else
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数
  TK_EOF,      // 入力の終わり
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
  TokenKind kind; // トークンの種類
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum {
  ND_ASSIGN, // =
  ND_LVAR,   // ローカル変数
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_NUM,    // 整数
  ND_RETURN, // return
  ND_IF,     // if
  ND_ELSE,     // if
} NodeKind;

// if構文の型
typedef struct Ifs Ifs;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_LVARの場合のみ使う
  Ifs *ifs;      // if構文の情報
};

struct Ifs {
  Node *cond;
  Node *then_body;
  Node *else_body;
  int counter;
};

// ローカル変数の型
typedef struct LVar LVar;
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

extern void error(char *fmt, ...);
extern void error_at(char *loc, char *fmt, ...);
extern Token *tokenize(char*);

extern void program();
extern Node *stmt();
extern Node *expr();
extern Node *assign();
extern Node *equality();
extern Node *relational();
extern Node *add();
extern Node *mul();
extern Node *unary();
extern Node *primary();

// -----------------------------------------------------------------------------
// Code Generator
// -----------------------------------------------------------------------------

extern void gen(Node *node);

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

extern char *user_input;
extern Token *token;
extern Node *code[100];
extern LVar *locals;
extern int label_counter_if;
