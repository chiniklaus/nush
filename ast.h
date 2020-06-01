#ifndef AST_H
#define AST_H

#include <stdlib.h>

typedef struct ast {
    char* op;
    struct ast* arg0;
    struct ast* arg1;
    char* str;
} ast;

ast* make_ast_str(char* s);
ast* make_ast_op(char* op, ast* a0, ast* a1);
void free_ast(ast* ast);

#endif
