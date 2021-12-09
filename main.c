#include "xcc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: 引数の個数が正しくありません", argv[0]);
  }

  // トークナイズしてパースする
  // 結果はcodeに格納される
  user_input = argv[1];
  token = tokenize(user_input);
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // プロローグ
  // 変数26個分の領域を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // 抽象構文木を下りながらコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果がスタックに一つあるので取り出す
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がRAXにあるので返り値にする
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
