#include "xcc.h"

// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

// トップレベルのノードの配列
Node *code[100];

// ローカル変数
LVar *locals = NULL;

// アセンブリのラベルに固有の名前をつけるためのラベル

// アセンブリラベルカウンタ: if
int label_counter_if = 0;

// アセンブリラベルカウンタ: while
int label_counter_while = 0;

// アセンブリラベルカウンタ: for
int label_counter_for = 0;

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
bool consume_reserved(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが識別子のときには、トークンを1つ読み進めて返す
// それ以外の場合には偽を返す
Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *tokenIdent = token;
  token = token->next;
  return tokenIdent;
}

// 次のトークンが期待している種類のときには、トークンを1つ読み進めて真を返す
// それ以外の場合には偽を返す
bool consume_token(TokenKind kind) {
  if (token->kind != kind)
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

// 文字が英数字またはアンダースコアなら真を返す
bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

// 文字列の先頭から/[a-zA-Z_][\w_]*/で構成された文字列部分を切り出す
char *match_ident_str(char *p) {
  int len = 1;

  if (!(('a' <= p[0] && p[0] <= 'z') ||
        ('A' <= p[0] && p[0] <= 'Z') ||
        (p[0] == '_'))) {
    return NULL;
  }

  while (p[len]) {
    if (is_alnum(p[len])) {
      len++;
    } else {
      break;
    }
  }

  if (len == 0) {
    return NULL;
  }

  char *s = (char*)malloc(sizeof(char) * len);
  memcpy(s, p, len);

  return s;
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
    if (strchr("+-*/(){}<>=;,", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
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

    // 識別子リテラル
    char *ident_str = match_ident_str(p);
    if (ident_str) {
      cur = new_token(TK_IDENT, cur, ident_str, 1);
      cur->len = strlen(ident_str);
      p += cur->len;
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

// 代入の構文木assignを生成する
// assign = equality ("=" assign)?
Node *syntax_assign() {
  Node *node = syntax_equality();
  if (consume_reserved("="))
    node = new_binary_node(ND_ASSIGN, node, syntax_assign());
  return node;
}

// 式の構文木exprを生成する
// expr = assign
Node *syntax_expr() {
  return syntax_assign();
}

// defn = ident "(" (ident ("," ident)*)? ")" block
Node *syntax_defn() {
  Token *tok = consume_ident();
  if (tok) {
    Node *node = new_node(ND_DEFN);
    node->defn = calloc(1, sizeof(DefineFn));
    node->defn->name = tok->str;
    node->defn->args = calloc(6, sizeof(int));
    expect("(");
    int argi = 0;
    while (!consume_reserved(")")) {
      if (argi > 0) {
        expect(",");
      }
      if (argi == DEFN_ARGS_MAX_COUNT) {
        error_at(
          token->str,
          "関数の引数は%d個以内に収めてください",
          DEFN_ARGS_MAX_COUNT
        );
      }
      Token *arg_ident = consume_ident();
      if (!arg_ident) {
        error_at(token->str, "引数名には識別子以外を記述できません");
      }
      node->defn->args[argi++] = arg_ident->str;
    }
    node->defn->args_count = argi;

    Node* block = syntax_block();
    if (!block) {
      error_at(token->str, "関数には本体ブロックが必要です");
    }
    node->defn->body = block;
    return node;
  }
  
  error_at(token->str, "トップレベルは関数定義のみ許可されます");
}

// block = "{" stmt* "}"
Node *syntax_block() {
  if (consume_reserved("{")) {
    Node *node = new_node(ND_BLOCK);

    // ブロック内の行ノード配列を動的に確保する
    Node **nodes = (Node**)calloc(STMT_ARR_ALLOC_UNIT, sizeof(Node));
    int current_size = STMT_ARR_ALLOC_UNIT;
    int count = 0;

    while (!consume_reserved("}")) {
      // 確保してあるサイズを超えたらリアロケート
      if (count >= current_size) {
        current_size += STMT_ARR_ALLOC_UNIT;
        nodes = (Node**)realloc(nodes, sizeof(Node) * current_size);
      }
      nodes[count++] = syntax_stmt();
    }
    node->stmts = nodes;
    node->stmts_len = count;
    return node;
  }

  return NULL;
}

// 文の構文木stmtを生成する
// stmt = expr ";"
//      | block
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
Node *syntax_stmt() {
  Node *node;

  Node *block_node = syntax_block();
  if (block_node)
    return block_node;

  if (consume_token(TK_IF)) {
    node = new_node(ND_IF);
    node->ifs = calloc(1, sizeof(IfNodes));
    expect("(");
    node->ifs->cond = syntax_expr();
    expect(")");
    node->ifs->then_body = syntax_stmt();

    if (consume_token(TK_ELSE)) {
      node->ifs->else_body = syntax_stmt();
    }

    node->ifs->counter = label_counter_if++;

    return node;
  }

  if (consume_token(TK_WHILE)) {
    node = new_node(ND_WHILE);
    node->whiles = calloc(1, sizeof(WhileNodes));
    expect("(");
    node->whiles->cond = syntax_expr();
    expect(")");
    node->whiles->body = syntax_stmt();

    node->whiles->counter = label_counter_while++;

    return node;
  }

  // "for" "(" expr? ";" expr? ";" expr? ")" stmt
  if (consume_token(TK_FOR)) {
    node = new_node(ND_FOR);
    node->fors = calloc(1, sizeof(ForNodes));
    expect("(");
    if (consume_reserved(";")) {
      node->fors->init = new_node(ND_EMPTY);
    } else {
      node->fors->init = syntax_expr();
      expect(";");
    }
    if (consume_reserved(";")) {
      node->fors->cond = new_node(ND_EMPTY);
    } else {
      node->fors->cond = syntax_expr();
      expect(";");
    }
    if (consume_reserved(")")) {
      node->fors->final = new_node(ND_EMPTY);
    } else {
      node->fors->final = syntax_expr();
      expect(")");
    }
    node->fors->body = syntax_stmt();

    node->fors->counter = label_counter_for++;

    return node;
  }

  if (consume_token(TK_RETURN)) {
    node = new_node(ND_RETURN);
    node->lhs = syntax_expr();
  } else {
    node = syntax_expr();
  }

  if (!consume_reserved(";"))
    error_at(token->str, "';'ではないトークンです");
  return node;
}

// 終端に到達するまでdefnを読み込む
void read_program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = syntax_defn();
  }
  code[i] = NULL;
}

// 一致比較の構文木equalityを生成する
// equality = relational ("==" relational | "!=" relational)*
Node *syntax_equality() {
  Node *node = syntax_relational();

  for (;;) {
    if (consume_reserved("=="))
      node = new_binary_node(ND_EQ, node, syntax_relational());
    else if (consume_reserved("!="))
      node = new_binary_node(ND_NE, node, syntax_relational());
    else
      return node;
  }
}

// 関係比較の構文木relationalを生成する
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *syntax_relational() {
  Node *node = syntax_add();

  for (;;) {
    if (consume_reserved("<"))
      node = new_binary_node(ND_LT, node, syntax_add());
    else if (consume_reserved("<="))
      node = new_binary_node(ND_LE, node, syntax_add());
    else if (consume_reserved(">"))
      node = new_binary_node(ND_LT, syntax_add(), node);
    else if (consume_reserved(">="))
      node = new_binary_node(ND_LE, syntax_add(), node);
    else
      return node;
  }
}

// 加算・減算の構文木addを生成する
// add = mul ("+" mul | "-" mul)*
Node *syntax_add() {
  Node *node = syntax_mul();

  for (;;) {
    if (consume_reserved("+"))
      node = new_binary_node(ND_ADD, node, syntax_mul());
    else if (consume_reserved("-"))
      node = new_binary_node(ND_SUB, node, syntax_mul());
    else
      return node;
  }
}

// 乗算・除算の構文木mulを生成する
// mul = unary ("*" unary | "/" unary)*
Node *syntax_mul() {
  Node *node = syntax_unary();

  for (;;) {
    if (consume_reserved("*"))
      node = new_binary_node(ND_MUL, node, syntax_unary());
    else if (consume_reserved("/"))
      node = new_binary_node(ND_DIV, node, syntax_unary());
    else
      return node;
  }
}

// 単項演算の構文木unaryを生成する
// unary = ("+" | "-")? unary
//       | primary
Node *syntax_unary() {
  if (consume_reserved("+"))
    return syntax_unary();
  if (consume_reserved("-"))
    return new_binary_node(ND_SUB, new_num_node(0), syntax_unary());
  return syntax_primary();
}

// 括弧で囲われた式あるいは数値の構文木primaryを生成する
// primary = num
//         | ident ("(" ( expr ("," expr)* )? ")")?
//         | "(" expr ")"
Node *syntax_primary() {
  // 括弧で囲われているなら式として再帰的に構文木を生成する
  if (consume_reserved("(")) {
    Node *node = syntax_expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    // 括弧があれば関数呼び出しとして処理
    if (consume_reserved("(")) {
      Node *node = new_node(ND_CALLFN);
      node->callfn = calloc(1, sizeof(CallFn));
      node->callfn->fnname = tok->str;
      node->callfn->args = calloc(CALLFN_ARGS_MAX_COUNT, sizeof(Node));
      // 引数を読み込む
      int i = 0;
      while (!consume_reserved(")")) {
        if (i > 0) {
          expect(",");
        }
        if (i == CALLFN_ARGS_MAX_COUNT) {
          error_at(
            token->str,
            "関数の引数は%d個以内に収めてください",
            CALLFN_ARGS_MAX_COUNT
          );
        }
        node->callfn->args[i++] = syntax_expr();
      }
      // TODO: ここでargsを再割り当てして切り詰めてもよさそう
      node->callfn->args_count = i;
      return node;
    }

    // 括弧がなければ変数として処理
    Node *node = new_node(ND_LVAR);
    node->lvar_name = tok->str;

    return node;
  }

  if (consume_reserved(";")) {
    // TODO: 空のノードにちゃんと対応する
    return new_node(ND_EMPTY);
  }

  // どれでもなければ数値ノードを返す
  return new_num_node(expect_number());
}
