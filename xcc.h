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
  TK_WHILE,    // while
  TK_FOR,      // for
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
  ND_ELSE,   // else
  ND_WHILE,  // while
  ND_FOR,    // for
  ND_BLOCK,  // { stmt* }
  ND_CALLFN, // 関数呼び出し
  ND_DEFN,   // 関数定義
  ND_EMPTY,
} NodeKind;

// if構文の型
typedef struct IfNodes IfNodes;

// while構文の型
typedef struct WhileNodes WhileNodes;

// for構文の型
typedef struct ForNodes ForNodes;

// 関数呼び出し構文の型
typedef struct CallFn CallFn;

// 関数定義構文の型
typedef struct DefineFn DefineFn;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
  NodeKind kind;      // ノードの型
  Node *lhs;          // 左辺
  Node *rhs;          // 右辺
  int val;            // kindがND_NUMの場合のみ使う
  int offset;         // kindがND_LVARの場合のみ使う
  char *lvar_name;    // ローカル変数の名前
  IfNodes *ifs;       // if構文の情報
  WhileNodes *whiles; // while構文の情報
  ForNodes *fors;     // for構文の情報
  Node **stmts;       // ブロックなどが持つ複数ノード
  int stmts_len;      // stmtsの長さ
  CallFn *callfn;     // 関数呼び出しの情報
  DefineFn *defn;     // 関数定義の情報
};

struct IfNodes {
  Node *cond;
  Node *then_body;
  Node *else_body;
  int counter;
};

struct WhileNodes {
  Node *cond;
  Node *body;
  int counter;
};

struct ForNodes {
  Node *init;
  Node *cond;
  Node *final;
  Node *body;
  int counter;
};

struct CallFn {
  char *fnname;
  Node **args;
  int args_count;
};

struct DefineFn {
  char *name;
  char **args;
  int args_count;
  Node *body;
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

extern void read_program();
extern Node *syntax_block();
extern Node *syntax_defn();
extern Node *syntax_stmt();
extern Node *syntax_expr();
extern Node *syntax_assign();
extern Node *syntax_equality();
extern Node *syntax_relational();
extern Node *syntax_add();
extern Node *syntax_mul();
extern Node *syntax_unary();
extern Node *syntax_primary();

// -----------------------------------------------------------------------------
// Code Generator
// -----------------------------------------------------------------------------

// ローカル変数の型（新）
typedef struct LocalVar LocalVar;
struct LocalVar {
  char *name; // 変数の名前
  int index;  // 変数が割り当てられるインデックス
};

// ローカル変数のコンスセルの型
typedef struct LocalVarCons LocalVarCons;
struct LocalVarCons {
  LocalVar *car;
  LocalVarCons *cdr;
};

extern void gen(Node *node);

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

extern char *user_input;
extern Token *token;
extern Node *code[100];
extern LVar *locals;
extern int label_counter_if;
extern int label_counter_while;
extern int label_counter_for;

// ブロック内の行ノードを保持する配列のアロケート単位
// パース時にこの単位でサイズを超えるたびにリアロケートする
#define STMT_ARR_ALLOC_UNIT 10

// 関数呼び出しの引数の最大個数
#define CALLFN_ARGS_MAX_COUNT 6

// 関数定義の引数の最大個数
#define DEFN_ARGS_MAX_COUNT 6
