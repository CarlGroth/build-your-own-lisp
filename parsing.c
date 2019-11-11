#include "mpc.h"

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}

typedef struct {
  int type;
  long num;
  int err;
} lval;

enum { LVAL_NUM, LVAL_ERR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

void lval_print(lval v) {
  switch (v.type) {
    case LVAL_NUM:
      printf("%li", v.num);
      break;
    case LVAL_ERR:
      if (v.err == LERR_DIV_ZERO) {
        printf("Error: Division by zero.");
      }
      if (v.err == LERR_BAD_OP) {
        printf("Error: Invalid operator.");
      }
      if (v.err == LERR_BAD_NUM) {
        printf("Error: Invalid Number.");
      }
      break;
  }
}

void lval_println(lval v) {
  lval_print(v);
  putchar('\n');
}

// typedef struct mpc_ast_t {
//   char* tag;
//   char* contents;
//   mpc_state_t state;
//   int children_num;
//   struct mpc_ast_t** children;
// } mpc_ast_t;

int number_of_nodes(mpc_ast_t* t) {
  if (t->children_num == 0) {
    return 1;
  }
  if (t->children_num >= 1) {
    int total = 1;
    for (int i = 0; i < t->children_num; i++) {
      total = total + number_of_nodes(t->children[i]);
    }
    return total;
  }
  return 0;
}

lval eval_op(lval x, char* op, lval y) {
  if (x.type == LVAL_ERR) {
    return x;
  }
  if (y.type == LVAL_ERR) {
    return y;
  }
  if (strcmp(op, "+") == 0) {
    return lval_num(x.num + y.num);
  }
  if (strcmp(op, "-") == 0) {
    return lval_num(x.num - y.num);
  }
  if (strcmp(op, "*") == 0) {
    return lval_num(x.num * y.num);
  }
  if (strcmp(op, "/") == 0) {
    return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
  }
  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  char* op = t->children[1]->contents;

  lval x = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  return x;
}

int main() {
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                     \
      number   : /-?[0-9]+/ ;                             \
      operator : '+' | '-' | '*' | '/' ;                  \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
            Number, Operator, Expr, Lispy);

  printf("Lispy Version 0.0.0.0.1\n");
  printf("Press Ctrl-C to Exit\n");

  while (1) {
    char* input = readline("lispy> ");
    mpc_result_t r;

    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      mpc_ast_t* a = r.output;

      mpc_ast_t* c = a->children[0];

      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    // printf("No you're a %s\n", input);
    free(input);
  }
  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;
}

//   mpc_parser_t* Adjective = mpc_or(4, mpc_sym("wow"), mpc_sym("many"),
//                                    mpc_sym("so"), mpc_sym("such"));

//   mpc_parser_t* Noun = mpc_or(5, mpc_sym("lisp"), mpc_sym("language"),
//                               mpc_sym("book"), mpc_sym("build"),
//                               mpc_sym("c"));

//   mpc_parser_t* Phrase = mpc_and(2, mpcf_strfold, Adjective, Noun, free);

//   mpc_parser_t* Doge = mpc_many(mpcf_strfold, Phrase);