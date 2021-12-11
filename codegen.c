#include "xcc.h"

// 代入の左辺値のアセンブリコードを出力する
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");
  
  // ノードが指す変数のアドレスをスタックに積む処理
  // 1. RBP（ベースレジスタ）の値（アドレス）をRAXにコピーする
  printf("  mov rax, rbp\n");
  // 2. RAXが保持しているアドレスを変数の位置に移動する
  printf("  sub rax, %d\n", node->offset);
  // 3. アドレスをスタックに積む
  printf("  push rax\n");
}

// 抽象構文木を解析してアセンブリコードを出力する
void gen(Node *node) {
  switch (node->kind) {
    // if
    case ND_IF:
      gen(node->ifs->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      if (node->ifs->else_body) {
        printf("  je .Lelse%d\n", node->ifs->counter);
      } else {
        printf("  je .Lend%d\n", node->ifs->counter);
      }
      gen(node->ifs->then_body);
      if (node->ifs->else_body) {
        printf("  jmp .Lend%d\n", node->ifs->counter);
        printf(".Lelse%d:\n", node->ifs->counter);
        gen(node->ifs->else_body);
      }
      printf(".Lend%d:\n", node->ifs->counter);
      return;
    // returnならスタックから値を取り出してRAXに入れる
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    // 数値なら値をスタックに積んで終わる
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    // 変数なら変数から値を参照して積む
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    // 代入なら右辺の値を左辺の変数（のアドレス宛）にセットする
    case ND_ASSIGN:
      // 左辺の変数の
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
  }

  // 左辺を出力
  gen(node->lhs);
  // 右辺を出力
  gen(node->rhs);

  // 演算のために値を2つ取り出す
  printf("  pop rdi\n");
  printf("  pop rax\n");

  // 演算子ごとにコードを出力
  switch (node->kind) {
    // ==
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    // !=
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    // <
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    // <=
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    // +
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    // -
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    // *
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    // /
    case ND_DIV:
      // RAXの64ビット値を128ビットに拡張してRDXとRAXにセットする
      printf("  cqo\n");
      // 1. 暗黙的にRDXとRAXを合わせたものを128ビット整数とみなす
      // 2. 1を引数のレジスタ（ここではRDI）の64ビット値で割る
      // 3. 2の商をRAXに、余りをRDXにセットする
      printf("  idiv rdi\n");
      break;
  }

  printf("  push rax\n");
}
