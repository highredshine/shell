#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define SIZE 1024
#define ARGC 64

int readinput(char *buffer) {
    ssize_t n = read(STDIN_FILENO, buffer, SIZE);
    if (n < 0) {
        printf("ERROR - name this error");
        return -1;
    } else if (n == 0) {
        exit(0);
    }
    return 0;
}

int parse(char *buffer, char **argv, char **redirection) {
    char * token = strtok(buffer, " \t\n");
    int argc = 0;
    while (token) {
        if (!strcmp(token, "<")) {
            if (!redirection[0]) {
                token = strtok(NULL, " \t\n");
                // if the filename is null, no redirectionion file is specified.
                if (!token) {
                    printf("ERROR - No redirectionion file specified.\n");
                    return -1;
                }
                redirection[0] = token;
            // if the redirection array already has input, return error.
            } else {
                printf("ERROR - Can’t have two input redirections on one line.\n");
                return -1;
            }
        } else if (!strcmp(token, ">")) {
            if (!redirection[1] && !redirection[2]) {
                token = strtok(NULL, " \t\n");
                if (!token) {
                    printf("ERROR - No redirectionion file specified.\n");
                    return -1;
                }
                redirection[1] = token;
            } else {
                printf("redirection output already in\n");
                return -1;
            }
        } else if (!strcmp(token, ">>")) {
            if (!redirection[1] && !redirection[2]) {
                token = strtok(NULL, " \t\n");
                if (!token) {
                    printf("ERROR - No redirectionion file specified.\n");
                    return -1;
                }
                redirection[2] = token;
            } else {
                printf("redirection output already in\n");
                return -1;
            }
        } else {
            argv[argc++] = token;
        }   
        token = strtok(NULL, " \t\n");
    }
    // after parsing, if all tokens are redirectionion commands or filenames,
    // it means no commands exist. Return error.
    if (argv[0] == 0) {
        printf("ERROR - No command.\n");
        return -1;
    }

    argv[argc] = NULL;
    return argc;
}

int redirect(char *redirection[]) {
    if (redirection[0]) {
        int std = close(STDIN_FILENO);
        if (std < 0) {
            printf("ERROR - input close");
            return -1;
        }
        int filedescriptor = open(redirection[0], O_RDONLY);
        if (filedescriptor < 0) {
            printf("ERROR - input open");
            return -1;
        }
    }
    if (redirection[1]) {
        int std = close(STDOUT_FILENO);
        if (std < 0) {
            printf("ERROR - input close");
            return -1;
        }
        int filedescriptor = open(redirection[1], O_CREAT | O_TRUNC | O_WRONLY);
        if (filedescriptor < 0) {
            printf("ERROR - input open");
            return -1;
        }
    }
    if (redirection[2]) {
        int std = close(STDOUT_FILENO);
        if (std < 0) {
            printf("ERROR - input close");
            return -1;
        }
        int filedescriptor = open(redirection[2], O_CREAT | O_APPEND | O_WRONLY);
        if (filedescriptor < 0) {
            printf("ERROR - input open");
            return -1;
        }
    }
    return 0;
}

int revert(char *redirection[], int input, int output) {
    if (redirection[0]) {
        if (dup2(input, STDIN_FILENO) < 0) {
            printf("ERROR - name this");
            return -1;
        }
    }
    if (redirection[1]) {
        if (dup2(output, STDOUT_FILENO) < 0) {
            printf("ERROR - name this");
            return -1;
        }
    }
    if (redirection[2]) {
        if (dup2(output, STDOUT_FILENO) < 0) {
            printf("ERROR - name this");
            return -1;
        }
    }
    return 0;
}

int execute(char *argv[]) {
    char *cmd = argv[0];
    // cd
    if (strcmp(cmd, "cd") == 0) {
        if (chdir(argv[1]) < 0) {
            printf("ERROR - cd");
            return -1;
        }
    // ln
    } else if (strcmp(cmd, "ln") == 0) {
        if (link(argv[1], argv[2]) < 0) {
            printf("ERROR - ln");
            return -1;
        }
    // rm
    } else if (strcmp(cmd, "rm") == 0) {
        if (unlink(argv[1]) < 0) {
            printf("ERROR - rm");
            return -1;
        }
    // exit
    } else if (strcmp(cmd, "exit") == 0) {
        exit(0);
    // fork
    } else {
        if (fork() < 0) {
            printf("ERROR - fork");
            return -1;
        } else {
            char *final = strrchr(argv[0], '/');
            if (final) {
                argv[0] = final + 1;
            }
            if (execv(cmd, argv) == 0) {
                printf("ERROR - execv");                
                exit(1);
                // return -1;
            }
        }
        wait(0);
    }
    return 0;
}

void print(int n, char *tokens[], char *redirection[]) {
    int i = 0;
    while (i < n) {
        printf("%d : %s \n", i, tokens[i]);
        i++;
    }
    i = 0;
    while (i < 3) {
        printf("%d : %s \n", i, redirection[i]);
        i++;
    }
}

int main() {
    char buffer[SIZE], *argv[ARGC];
    while(1) {
        // prompt vs noprompt
        #ifdef PROMPT
            printf("33sh> ");
            fflush(stdout);
        #endif
        // reset arrays for command call
        memset(buffer,'\0', SIZE);
        memset(argv,'\0', sizeof(argv));
        char *redirection[3] = {NULL, NULL, NULL};
        int input = dup(STDIN_FILENO), output = dup(STDOUT_FILENO);
        // read command line
        int r = readinput(buffer);
        if (r < 0) {
            continue;
        }
        // no input: new loop.
        if (buffer[0] == '\n') {
            continue;
        // yes input: main code
        } else {
            // parse the command line
            if (parse(buffer, argv, redirection) < 0) {
                continue;
            }
            if (redirect(redirection) < 0) {
                continue;
            }
            if (execute(argv) < 0) {
                continue;
            }
            if (revert(redirection, input, output) < 0) {
                continue;
            }
            // print(n, tokens,redirection);
        }
    }
    return 0;
}