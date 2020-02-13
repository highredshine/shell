#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void parse(char *buffer, char *tokens[], char *argv[], char *redirect_input[],
           char *redirect_output[], char *redirect_append[]) {
    char *tok = strtok(buffer, " \t");
    int tok_idx = 0;

    while (tok != NULL) {
        tokens[tok_idx] = tok;
        tok_idx++;
        tok = strtok(NULL, " \t");
    }

    redirect_input[0] = "<";
    redirect_input[1] = NULL;

    redirect_output[0] = ">";
    redirect_output[1] = NULL;

    redirect_append[0] = ">>";
    redirect_append[1] = NULL;

    for (int i = 0; i < tok_idx; i++) {  // looping though for tokens
        char *curr = tokens[i];

        if (strcmp(curr, "<") == 0) {
            if (redirect_input[1] == NULL) {
                if (i == tok_idx - 1) {
                    fprintf(stderr, "ERROR: no file to redirect for \n");
                }

                if ((strcmp(tokens[i + 1], "<") == 0) |
                    (strcmp(tokens[i + 1], ">") == 0) |
                    (strcmp(tokens[i + 1], ">>") == 0)) {
                    fprintf(
                        stderr,
                        "ERROR: redirection symbol not followed by file \n");
                }

                redirect_input[1] = tokens[i + 1];
                tokens[i] = "to_remove";  // set as some arbitrary value
                tokens[i + 1] = "to_remove";
            } else {
                fprintf(
                    stderr,
                    "ERROR - Can’t have two input redirects on one line. \n");
            }

        } else if (strcmp(curr, ">") == 0) {
            if (redirect_output[1] == NULL) {
                if (i == tok_idx - 1) {
                    fprintf(stderr, "ERROR: no file to redirect for \n");
                }

                if ((strcmp(tokens[i + 1], "<") == 0) |
                    (strcmp(tokens[i + 1], ">") == 0) |
                    (strcmp(tokens[i + 1], ">>") == 0)) {
                    fprintf(
                        stderr,
                        "ERROR: redirection symbol not followed by file \n");
                }

                redirect_output[1] = tokens[i + 1];
                tokens[i] = "to_remove";
                tokens[i + 1] = "to_remove";
            } else {
                perror("parse");
            }
        } else if (strcmp(curr, ">>") == 0) {
            if (redirect_append[1] == NULL) {
                if (i == tok_idx - 1) {
                    fprintf(stderr, "ERROR: no file to redirect for \n");
                }

                if ((strcmp(tokens[i + 1], "<") == 0) |
                    (strcmp(tokens[i + 1], ">") == 0) |
                    (strcmp(tokens[i + 1], ">>") == 0)) {
                    fprintf(
                        stderr,
                        "ERROR: redirection symbol not followed by file \n");
                }

                redirect_append[1] = tokens[i + 1];
                tokens[i] = "to_remove";
                tokens[i + 1] = "to_remove";
            } else {
                perror("parse");
            }
        }
    }

    int i, argv_idx;
    argv_idx = 0;
    for (i = 0; i < tok_idx; i++) {
        if (strcmp(tokens[i], "to_remove") != 0) {
            argv[argv_idx] = tokens[i];
            argv_idx++;
        }
    }

    argv[argv_idx] = NULL;
}

void commands(char *tokens[], char *argv[], char *redirect_input[],
              char *redirect_output[],
              char *redirect_append[]) {  // error checking!!
    char *full_path = argv[0];

    if (strrchr(full_path, '/') != NULL) {
        argv[0] = strrchr(full_path, '/') + 1;
    }

    char *command = full_path;

    int check_val;
    if (strcmp(command, "cd") == 0) {
        check_val = chdir(tokens[1]);
        if (check_val != 0) {
            if (tokens[1] == NULL) {
                fprintf(stderr, "cd: syntax error \n");
            } else {
                perror("commands");
            }
        }

    } else if (strcmp(command, "ln") == 0) {
        check_val = link(tokens[1], tokens[2]);
        if (check_val == -1) {
            perror("commands");
        }
    } else if (strcmp(command, "rm") == 0) {
        check_val = unlink(tokens[1]);
        if (check_val == -1) {
            perror("commands");
        }
    } else if (strcmp(command, "exit") == 0) {
        exit(0);
    } else {
        pid_t fork_val = fork();

        if (fork_val == -1) {
            perror("commands");
            exit(1);
        } else if (fork_val == 0) {
            if (redirect_output[1] != NULL) {
                int close_val = close(STDOUT_FILENO);
                if (close_val < 0) {
                    perror("close");
                    exit(1);
                }
                int open_val = open(redirect_output[1],
                                    O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (open_val < 0) {
                    perror("open");
                    exit(1);
                }
            }

            if (redirect_input[1] != NULL) {
                int close_val = close(STDIN_FILENO);
                if (close_val < 0) {
                    perror("close");
                    exit(1);
                }
                int open_val = open(redirect_input[1], O_RDONLY, 0400);
                if (open_val < 0) {
                    perror("open");
                    exit(1);
                }
            }

            if (redirect_append[1] != NULL) {
                int close_val = close(STDOUT_FILENO);
                if (close_val < 0) {
                    perror("close");
                    exit(1);
                }
                int open_val =
                    open(redirect_append[1], O_RDWR | O_CREAT | O_APPEND, 0666);
                if (open_val < 0) {
                    perror("open");
                    exit(1);
                }
            }

            int exec_val = execv(full_path, argv);

            if (exec_val == 0) {
                perror("commands");
                exit(1);
            }
        }

        wait(0);
    }
}

int main() {
    while (1) {
        char buffer[1024];
        char *tokens[1024];
        char *argv[1024];

        char *redirect_input[2];
        char *redirect_output[2];
        char *redirect_append[2];

        memset(buffer, '\0', 1024);
        memset(tokens, '\0', sizeof(tokens));

#ifdef PROMPT
        if (printf("33sh> ") < 0) {
            fprintf(stderr, "error '\n'");
            return -1;
        }

        fflush(stdout);
#endif

        ssize_t input = read(STDIN_FILENO, buffer, 1024);

        if (input == 0) {
            return 0;
        } else if (input == -1) {
            perror("read");
            return -1;
        } else if (buffer[0] == '\n') {
            continue;
        } else {
            buffer[input - 1] = '\0';
            parse(buffer, tokens, argv, redirect_input, redirect_output,
                  redirect_append);

            if (tokens[0] == NULL) {
                continue;
            }

            commands(tokens, argv, redirect_input, redirect_output,
                     redirect_append);
        }
    }
}