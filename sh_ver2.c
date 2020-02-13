#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define SIZE 1024
#define ARGC 64

int main() {
    char buffer[SIZE], *tokens[ARGC], *argv[ARGC];
    char *token, *curr, *final;
    int lenToken, lenArgv, i;
    while(1) {
        // prompt vs noprompt
        #ifdef PROMPT
            printf("33sh> ");
            fflush(stdout);
        #endif
        // reset arrays
        memset(buffer,'\0', SIZE);
        memset(tokens,'\0', sizeof(tokens));
        memset(argv,'\0', sizeof(argv));
        char *redirect[3] = {NULL, NULL, NULL};
        // read command line and count how many = n
        ssize_t n = read(STDIN_FILENO, buffer, SIZE);
        // no input: new loop
        if (buffer[0] == '\n') {
            continue;
        // yes input: n = 0, negative, or positive
        } else if (n == 0) {
            exit(0);
        } else if (n < 0) {
            printf("invalid input");
            exit(1);
        // main code:
        } else {
            /* Parse */
            // tokenize and store in token array 
            token = strtok(buffer, " \t");
            lenToken = 0;
            while (token) {
                tokens[lenToken++] = token;
                token = strtok(NULL, " \t");
            }
            // handle redirection
            for (i = 0; i < lenToken; i++) {
                curr = tokens[i];
                if (!strcmp(curr, "<")) {
                    if (!redirect[0]) {
                        redirect[0] = tokens[i+1];
                        tokens[i] = "remove";
                        tokens[i+1] = "remove";
                    } else {
                        perror("redirection");
                    }
                } else if (!strcmp(curr, ">")) {
                    if (!redirect[1] && !redirect[2]) {
                        redirect[1] = tokens[i+1];
                        tokens[i] = "remove";
                        tokens[i+1] = "remove";
                    } else {
                        perror("redirection");
                    }
                } else if (!strcmp(curr, ">>")) {
                    if (!redirect[1] && !redirect[2]) {
                        redirect[2] = tokens[i+1];
                        tokens[i] = "remove";
                        tokens[i+1] = "remove";
                    } else {
                        perror("redirection");
                    }
                }
            }
            // copy tokens to arguments
            lenArgv = 0;
            for (i = 0; i < lenToken; i++) {
                if (strcmp(tokens[i], "remove")) {
                    argv[lenArgv++] = tokens[i];
                }
            }
            argv[lenArgv] = NULL;
            // get pointer to final path of the redirection
            final = strrchr(argv[0],'/');
            if (final) {
                argv[0] = final++;
            }
            
            /* Execute */
            char *cmd = argv[0];
            // cd
            if (!strcmp(cmd, "cd")) {
                if (chdir(argv[1]) < 0) {
                    perror("cd");
                    exit(1);
                }
            // ln
            } else if (!strcmp(cmd, "ln")) {
                if (link(argv[1], argv[2]) < 0) {
                    perror("ln");
                    exit(1);
                }
            // rm
            } else if (!strcmp(cmd, "rm")) {
                if (unlink(argv[1]) < 0) {
                    perror("rm");
                    exit(1);
                }
            // exit
            } else if (!strcmp(cmd, "exit")) {
                exit(0);
            // redirection
            } else {
                if (fork() < 0) {
                    perror("fork");
                    exit(1);
                } else {
                    if (redirect[0] != NULL) {
                        open(redirect[0], O_RDONLY, 0600);
                    }
                    if (redirect[1] != NULL) {
                        open(redirect[1], O_WRONLY | O_CREAT, 0400);
                    }
                    if (redirect[2] != NULL) {
                        open(redirect[2], O_RDWR | O_TRUNC, 0400);
                    }
                    if (!execv(cmd, argv)) {
                        perror("redirect");
                        exit(1);
                    }
                }
                wait(0);
            }
        }
    }
    return 0;
}