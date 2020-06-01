#include "vec.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "svec.h"
#include "ast.h"
#include "parse.h"

vec*
read_tokens(long* n, int fd) {

    long cap = 16;
    char* buf = malloc(cap);
    long ii = 0; //bytes read
    
    while (1) {
        if ((ii * 8) >= cap) {
            cap *= 2;
            buf = realloc(buf,cap);
        }
        int result = read(fd,buf + ii,1);

        if (result == 0) {
            exit(0);
        }

        if (buf[ii] == '\n') {
            break;
        }
        ii++;
    }

    *n = ii;

    vec* v = make_vec();
    for (int j = 0; j < ii; j++) {
        push_vec(v, buf[j]);
    }
    free(buf);
    return v;
}

void
add_to_buf(char** buf, long n, vec* vv) {
    push_vec(vv,'\0');
    buf[n] = strdup(vv->data);
}

void
update(long* state, char bj, int* counter1, int* counter2, long* ts) {
    //there are 6 states
    //0 start
    //1 spaces
    //2 c token
    //3 ; < > token
    //4 & && token
    //5 waiting for next token
    //6 | || token

    if (*state == 0) {
        switch(bj) {
            case ' ' :
                *state = 1;
                break;
            case ';' :
                *state = 3;
                break;
            case '<' :
                *state = 3;
                break;
            case '>' :
                *state = 3;
                break;
            case '&' :
                *state = 4;
                *counter1 = 1;
                break;
            case '|' :
                *state = 6;
                *counter2 = 1;
                break;
            default:
                *state = 2;
        }
    }

    else if (*state == 1) {
        switch(bj) {
            case ' ' :
                *state = 1;
                break;
            case ';' :
                *state = 3;
                break;
            case '<' :
                *state = 3;
                break;
            case '>' :
                *state = 3;
                break;
            case '&' :
                *counter1 = 1;
                *state = 4;
                break;
            case '|' :
                *counter2 = 1;
                *state = 6;
                break;
            default:
                *state = 2;
        }
    }

    else if (*state == 2) {
        switch(bj) {
            case ' ' :
                *state = 5;
                break;
            case ';' :
                *ts = *ts + 1;
                *state = 3;
                break;
            case '<' :
                *ts = *ts + 1;
                *state = 3;
                break;
            case '>' :
                *ts = *ts + 1;
                *state = 3;
                break;
            case '&' :
                *counter1 = 1;
                *ts = *ts + 1;
                *state = 4;
                break;
            case '|' :
                *counter2 = 1;
                *ts = *ts + 1;
                *state = 6;
                break;
            default:
                *state = 2;
        }
    }

    else if (*state == 3) {
        switch(bj) {
            case ' ' :
                *state = 5;
                break;
            case ';' :
                *ts = *ts + 1;
                break;
            case '<' :
                *ts = *ts + 1;
                break;
            case '>' :
                *ts = *ts + 1;
                break;
            case '&' :
                *counter1 = 1;
                *ts = *ts + 1;
                *state = 4;
                break;
            case '|' :
                *counter2 = 1;
                *ts = *ts + 1;
                *state = 6;
                break;
            default:
                *ts = *ts + 1;
                *state = 2;
        }
    }

    else if (*state == 4) {
        switch(bj) {
            case ' ' :
                *state = 5;
                *counter1 = 0;
                break;
            case ';' :
                *ts = *ts + 1;
                *state = 3;
                *counter1 = 0;
                break;
            case '<' :
                *ts = *ts + 1;
                *state = 3;
                *counter1 = 0;
                break;
            case '>' :
                *ts = *ts + 1;
                *state = 3;
                *counter1 = 0;
                break;
            case '&' :
                *counter1 = *counter1 + 1;
                if (*counter1 == 3) {
                    *ts = *ts + 1;
                    *counter1 = 1;
                }
                break;
            case '|' :
                *ts = *ts + 1;
                *state = 6;
                *counter1 = 0;
                *counter2 = 1;
                break;
            default:
                *ts = *ts + 1;
                *state = 2;
                *counter1 = 0;
        }
    }

    else if (*state == 5) {
        switch(bj) {
            case ' ' :
                *state = 5;
                break;
            case ';' :
                *ts = *ts + 1;
                *state = 3;
                break;
            case '<' :
                *ts = *ts + 1;
                *state = 3;
                break;
            case '>' :
                *ts = *ts + 1;
                *state = 3;
                break;
            case '&' :
                *counter1 = 1;
                *ts = *ts + 1;
                *state = 4;
                break;
            case '|' :
                *counter2 = 1;
                *ts = *ts + 1;
                *state = 6;
                break;
            default:
                *ts = *ts + 1;
                *state = 2;
        }
    }


    else if (*state == 6) {
        switch(bj) {
            case ' ' :
                *state = 5;
                *counter2 = 0;
                break;
            case ';' :
                *ts = *ts + 1;
                *state = 3;
                *counter2 = 0;
                break;
            case '<' :
                *ts = *ts + 1;
                *state = 3;
                *counter2 = 0;
                break;
            case '>' :
                *ts = *ts + 1;
                *state = 3;
                *counter2 = 0;
                break;
            case '|' :
                *counter2 = *counter2 + 1;
                if (*counter2 == 3) {
                    *ts = *ts + 1;
                    *counter2 = 1;
                }
                break;
            case '&' :
                *ts = *ts + 1;
                *state = 4;
                *counter1 = 1;
                *counter2 = 0;
                break;
            default:
                *ts = *ts + 1;
                *state = 2;
                *counter2 = 0;
        }
    } 
}

vec*
find_token(int i, char* b, long size) {
    long* state = malloc(sizeof(long));
    *state = 0;

    int* counter1 = malloc(sizeof(int)); //counter for &
    *counter1 = 0;

    int* counter2 = malloc(sizeof(int)); //counter for |
    *counter2 = 0;

    long* token_seen = malloc(sizeof(long));
    *token_seen = 0;

    vec* vv = make_vec();
    int j = 0;

    while (j < size) {
        update(state,b[j],counter1,counter2,token_seen);
        if (*token_seen == i && b[j] != ' ' && b[j] != '\v' && b[j] != '\t') {
            push_vec(vv, b[j]);
        }
        j++;
    }
    vv->lilsth = *token_seen; //the total number of tokens
    free(state);
    free(counter1);
    free(counter2);
    free(token_seen);
    return vv;
}

ast*
to_ast(vec* vv) {
    vec* dummy = find_token(0, vv->data, vv->size);
    //while i call update in find_token, the function put the number
    //of tokens into the size of the vec. We use that to find the size
    long size_of_buffer = dummy->lilsth + 1; //find out the size of the buffer
    char** buf = malloc(size_of_buffer * sizeof(char*));

    for (int i = 0; i < size_of_buffer; i++) {
        vec* token = find_token(i, vv->data, vv->size);
        add_to_buf(buf, i, token);
        free_vec(token);
    }

    svec* toks = make_svec();

    for (int j = 0; j < size_of_buffer; j++) {
        svec_push_back(toks,buf[j]);
    }

    ast* ast = parse(toks);
    for (int m = 0; m < size_of_buffer; m++) {
        free(buf[m]);
    }
    free(buf);
    free_svec(toks);
    free_vec(dummy);
    return ast;
}

void
check_rv(int rv) {
    if (rv == -1) {
        perror("fail");
        exit(1);
    }
}

int
execute(ast* ast) {

    //exit code
    int status;

    if (ast->arg0 == NULL && ast->arg1 == NULL  && strcmp("exit", ast->str) == 0) {
        exit(0);
    }
    else if (ast->arg0 == NULL && ast->arg1 == NULL  && strcmp("cd", ast->str) == 0) {
        chdir(getenv("HOME"));
    }
    else if (strcmp("cd", ast->str) == 0 && ast->arg0 != NULL) {
        chdir(ast->arg0->str);
    }
    else if (strcmp("", ast->op) == 0){
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed.");
        }
        else if (pid == 0) {
            svec* v = make_svec();
            while (1) {
                if (ast->arg0 == NULL) {
                    svec_push_back(v, ast->str);
                    break;
                }
                svec_push_back(v, ast->str);
                ast = ast->arg0;
            }
            
            char* args[v->size + 1];
            for (int i = 0; i < v->size; i++) {
                args[i] = v->data[i];
            }
            args[v->size] = '\0';
            if (strcmp(args[0], "true") == 0) {
                exit(0);
            }
            if (strcmp(args[0], "false") == 0) {
                exit(1);
            }            
            execv(args[0], args);
            printf("ran program %s  \n", args[0]);
            printf("execv error : %d  \n", errno);
            exit(0);
        }
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
    else {
        if (strcmp(";", ast->op) == 0) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed.");
            }
            else if (pid == 0) {
                execute(ast->arg0);
                execute(ast->arg1); 
            }
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
        if (strcmp("&", ast->op) == 0) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed.");
            }
            else if (pid == 0) {
                execute(ast->arg0);
                exit(0);
            }
            return 0;
        }
        if (strcmp("<", ast->op) == 0) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed.");
            }
            else if (pid == 0) {
                if (ast->arg1 != NULL) {
                    int fd = open(ast->arg1->str, O_RDWR);
                    close(0);
                    dup(fd);
                    close(fd);
                    execute(ast->arg0);
                }
                exit(0);
            }
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
        if (strcmp(">", ast->op) == 0) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed.");
            }
            else if (pid == 0) {
                if (ast->arg0 != NULL) {
                    int fd = open(ast->arg1->str, O_CREAT | O_APPEND | O_WRONLY, 0644);
                    close(1);
                    dup(fd);
                    close(fd);
                    execute(ast->arg0);
                    exit(0);
                }
            }
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
        if (strcmp("|", ast->op) == 0) {
            pid_t p = fork();
            if (p < 0) {
                perror("fork failed.");
                return -1;
            }
            else if (p == 0) {
                int rv;
                int pipe_fds[2];
                rv = pipe(pipe_fds);
                check_rv(rv);
                int p_read = pipe_fds[0];
                int p_write = pipe_fds[1];
                int cpid;

                if ((cpid = fork())) {
                    if (cpid == -1) {
                        perror("fork");
                        return -1;
                    }

                    //in the parent
                    wait(NULL);
                    close(0);
                    close(p_write);
                    dup(p_read);
                    close(p_read);
                    execute(ast->arg1);
                    exit(0);
                }
                else {
                    //in the child
                    close(1);
                    close(p_read);
                    dup(p_write);
                    close(p_write);
                    execute(ast->arg0);
                    exit(0);
                }
                close(p_read);
                close(p_write);
                exit(0);
            }
            waitpid(p, &status, 0);
            return WEXITSTATUS(status);
        }
        if (strcmp("||", ast->op) == 0) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed.");
            }
            else if (pid == 0) {
                int sta = execute(ast->arg0);
                if (sta != 0) {
                    int s = execute(ast->arg1);
                    exit(s);
                }
                exit(0);
            }
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
        if (strcmp("&&", ast->op) == 0) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed.");
            }
            else if (pid == 0) {
                int s1 = execute(ast->arg0);
                if (s1 != 0) {
                    exit(s1);
                }
                else {
                    int s2 = execute(ast->arg1);
                    if (s2 != 0) {
                        exit(s2);
                    }
                    else {
                        exit(0);
                    }
                }
            }
            waitpid(pid, &status, 0);
            if (WEXITSTATUS(status) != 0) {
                return WEXITSTATUS(status);
            }
            else {
                exit(WEXITSTATUS(status));
            }
        }
    }
}

int main(int argc, char* argv[]) {
    char* header = "nush$ ";
    char cmd[256];

    if (argc == 1) {
        while (1) {
            long nn;
            write(1, header, 6);
            vec* vv = read_tokens(&nn, 0);
            ast* a = to_ast(vv);
            execute(a);
            free_vec(vv);
            free_ast(a);
        }
    }
    else {
        int fd = open(argv[1], O_RDONLY);
        while (1) {
            long mm;
            vec* vn = read_tokens(&mm, fd);
            ast* b = to_ast(vn);
            execute(b);
            free_vec(vn);
            free_ast(b);
        }
    }
}

