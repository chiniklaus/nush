#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "ast.h"
#include "parse.h"

int
streq(const char* aa, const char* bb) {
    return strcmp(aa, bb) == 0;
}

int
find_first_index(svec* toks, const char* tt) {
    for (int ii = 0; ii < toks->size; ++ii) {
        if (streq(toks->data[ii], tt)) {
            return ii;
        }
    }

    return -1;
}

int
contains(svec* toks, const char* tt) {
    return find_first_index(toks, tt) >= 0;
}

svec*
slice(svec* xs, int i0, int i1) {
    svec* ys = make_svec();
    for (int ii = i0; ii < i1; ++ii) {
        svec_push_back(ys, xs->data[ii]);
    }
    return ys;
}

void
add_argument(ast* a, char* s) {
    if (a->arg0 == NULL) {
        a->arg0 = make_ast_str(s);
    }
    else {
        add_argument(a->arg0, s);
    }
}

ast*
parse(svec* toks) {
    if (toks->size == 0) {
        return make_ast_str("");
    }

    if (toks->size == 1) {
        if (strcmp(toks->data[0], "exit") == 0) {
            return make_ast_str("exit");
        }
        if (strcmp(toks->data[0], "cd") == 0) {
            return make_ast_str("cd");
        }
        char* new_str;
        new_str = malloc(5 + strlen(toks->data[0]));
        new_str[0] = '\0';

        if (strcmp(toks->data[0], "wc") == 0 || strcmp(toks->data[0], "sort") == 0 || strcmp(toks->data[0], "tac") == 0) {
            strcat(new_str, "/usr/bin/");
        }
        else if (strcmp(toks->data[0], "ls") == 0 || strcmp(toks->data[0], "pwd") == 0) {
            strcat(new_str, "/bin/");
        }
        strcat(new_str, toks->data[0]);
        ast* g = make_ast_str(new_str);

        return g;
    }

    if (toks->size > 1 && strcmp(toks->data[0], "cd") == 0) {
        ast* g = make_ast_str("cd");

        for (int jj = 1; jj < toks->size; jj++) {
            add_argument(g, toks->data[jj]);
        }

        return g;
    }

    char* ops[] = {";", "<", ">", "|", "&", "||", "&&"};

    for (int ii = 0; ii < 7; ii++) {
        char* op = ops[ii];

        if (contains(toks, op)) {
            int jj = find_first_index(toks, op);
            svec* xs = slice(toks,0,jj);
            svec* ys = slice(toks,jj+1,toks->size);
            ast* g1 = parse(xs);
            ast* g2 = parse(ys);
            ast* ast = make_ast_op(op, g1 ,g2);
            free_svec(xs);
            free_svec(ys);
            return ast;
        }
    }
    
    char* new_str;
    new_str = malloc(9 + strlen(toks->data[0]));
    new_str[0] = '\0';
    if (strcmp(toks->data[0], "wc") == 0 || strcmp(toks->data[0], "sort") == 0 || strcmp(toks->data[0], "perl") == 0 || strcmp(toks->data[0], "tail") == 0 || strcmp(toks->data[0], "tac") == 0) {
        strcat(new_str, "/usr/bin/");
    }
    else {
        strcat(new_str, "/bin/");
    }
    strcat(new_str, toks->data[0]);
    ast* g = make_ast_str(new_str);

    for (int jj = 1; jj < toks->size; jj++) {
        add_argument(g, toks->data[jj]);
    }
    free(new_str);
    return g;

    fprintf(stderr, "Invalid token stream");
    exit(1);
}
