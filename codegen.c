#include "xcc.h"

// 抽象構文木を解析してアセンブリコードを出力する
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
