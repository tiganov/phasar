#include <stdio.h>
#include <stdlib.h>

void foo(FILE *f) { fclose(f); }

int main(int argc, char **argv) {
  FILE *f;
  int x;

  x = 3;

  fscanf(f, "%d", &x);

  f = fopen(argv[1], "a");

  foo(f);

  return 0;
}