#include <stdio.h>

int noargs() { printf("noargs: OK\n"); return 0; }
int square(int x) { printf("%d\n", x * x); return 0; }
int sum(int x, int y) { printf("%d\n", x + y); return 0; }
