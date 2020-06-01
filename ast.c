#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

ast*
make_ast_str(char* s) {
    ast* a = malloc(sizeof(ast));
    a->op = "";
    a->arg0 = NULL;
    a->arg1 = NULL;
    a->str = strdup(s);
    return a;
}

ast*
make_ast_op(char* op, ast* a0, ast* a1) {
    ast* a = malloc(sizeof(ast));
    a->op = strdup(op);
    a->arg0 = a0;
    a->arg1 = a1;
    a->str = "";
    return a;
}

void free_ast(ast* ast) {
    if (ast) {
        if (ast->arg0) {
            free_ast(ast->arg0);
        }
        if (ast->arg1) {
            free_ast(ast->arg1);
        }
        if (ast->str && strcmp("", ast->str) != 0) {
            free(ast->str);
        }
        if (ast->op && strcmp("", ast->op) != 0) {
            free(ast->op);
        }
        free(ast);
    }
}

