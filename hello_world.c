#include <stdio.h>

static char input[2048];

int main(int argc, char **argv) {
  printf("Lispy Version 0.0.0.0.1\n");
  printf("Press Ctrl-C to Exit\n");

  while (1) {
    fputs("lispy> ", stdout);

    fgets(input, 2048, stdin);

    printf("No you're a %s", input);
  }
  return 0;
}