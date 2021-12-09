#include "xcc.h"

// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 位置表示機能付きのエラー報告関数
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

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて真を返す
// それ以外の場合には偽を返す
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める
// それ以外の場合にはエラーを報告する
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "\"%s\"ではありません", op);
  token = token->next;
}

// 次のトークンが数値のときには、トークンを1つ読み進めてその値を返す
// それ以外の場合にはエラーを報告する
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数値ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

// トークンが終端なら真を返す
bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを生成し、現在のトークンの次のトークンとして設定する
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// 文字列pが文字列qで始まっていれば真を返す
// それ以外の場合には偽を返す
// ※単純な一致検査だとpの残りのコードまで対象になってしまう
bool starts_with(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

// 入力文字列pをトークナイズして返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白を無視する
    if (isspace(*p)) {
      p++;
      continue;
    }

    // 2文字の演算子
    if (starts_with(p, "==") ||
        starts_with(p, "!=") ||
        starts_with(p, "<=") ||
        starts_with(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // 1文字の演算子と括弧
    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // 整数リテラル
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "無効なトークンです");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// 新しいトークンを生成する
Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

// 新しい二項演算トークンを生成する
Node *new_binary_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// 新しい数値トークンを生成する
Node *new_num_node(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

// 式の構文木exprを生成する
// expr = equality
Node *expr() {
  return equality();
}

// 一致比較の構文木equalityを生成する
// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_binary_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary_node(ND_NE, node, relational());
    else
      return node;
  }
}

// 関係比較の構文木relationalを生成する
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_binary_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary_node(ND_LE, add(), node);
    else
      return node;
  }
}

// 加算・減算の構文木addを生成する
// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_binary_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// 乗算・除算の構文木mulを生成する
// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// 単項演算の構文木unaryを生成する
// unary = ("+" | "-")? unary
//       | primary
Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary_node(ND_SUB, new_num_node(0), unary());
  return primary();
}

// 括弧で囲われた式あるいは数値の構文木primaryを生成する
// primary = "(" expr ")" | num
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  return new_num_node(expect_number());
}