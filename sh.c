#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_LENGTH 1024
#define MAX_ARGC 64
#define REDIRECT_TYPES 3

int parse(char *buffer, char *argv[], char *file_paths[]) {
    char *token = strtok(buffer, " \t\n");
    int argc = 0;
    while (token) {
        // check for redirect input
        if (strcmp(token, "<") == 0) {
            if (file_paths[0] == '\0') {
                // get file path
                token = strtok(NULL, " \t\n");
                // if the filename is null, no redirectionion file is specified.
                if (token == NULL) {
                    printf("ERROR - No redirection file specified.\n");
                    return -1;
                }
                file_paths[0] = token;
            // if the redirection array already has input, return error.
            } else {
                printf("ERROR - Can’t have two input redirections on one line.\n");
                return -1;
            }
        // check for redirect output: same syntax
        } else if (strcmp(token, ">") == 0) {
            if (file_paths[1] == '\0' && file_paths[2] == '\0') {
                token = strtok(NULL, " \t\n");
                if (token == NULL) {
                    printf("ERROR - No redirection file specified.\n");
                    return -1;
                }
                file_paths[1] = token;
            } else {
                printf("ERROR - Can’t have two output redirections on one line.\n");
                return -1;
            }
        // check for redirect append
        } else if (strcmp(token, ">>") == 0) {
            if (file_paths[1] == '\0' && !redirection[2] = '\0') {
                token = strtok(NULL, " \t\n");
                if (token == NULL) {
                    printf("ERROR - No redirection file specified.\n");
                    return -1;
                }
                redirection[2] = token;
            } else {
                printf("ERROR - Can’t have two output redirections on one line.\n");
                return -1;
            }
        } else {
            argv[argc] = token;
            argc++;
        }   
        token = strtok(NULL, " \t\n");
    }
    // if all tokens are redirection commands or filenames, it means no commands exist.
    if (argv[0] == 0) {
        printf("ERROR - No command.\n");
        return -1;
    }

    // possbily not needed: test - argv[argc] = '\0';
    
    return argc;
}

int execute(char *argv[]) {
    char *cmd = argv[0];
    if (strcmp(command, "cd") == 0) {
        if (chdir(argv[1]) < 0) {
            if (argv[1] == NULL) {
                printf("cd: syntax error\n");
            } else {
                printf("cd: no such file or directory: %s\n", argv[1]);
            }
            return -1;
        }
    } else if (strcmp(command, "ln") == 0) {
        if (link(tokens[1], tokens[2]) < 0) {
            printf("ln: error\n");
            return -1;
        }
    } else if (strcmp(command, "rm") == 0) {
        check_val = 
        if (unlink(tokens[1]) < 0) {
            printf("rm: error\n");
            return -1;
        }
    } else if (strcmp(command, "exit") == 0) {
        exit(0);
    } else {
        return 1;
    }
    return 0;
}

int redirect(char *argv[], char *file_paths[]) {
    char *cmd = argv[0];
    // find last part of the command
    char *last = strrchr(cmd, '/');
    if (last) {
        argv[0] = last + 1;
    }
    // fork process
    if (fork() < 0) {
        printf("ERROR - fork process incorrect.\n");
        exit(1);
        return -1;
    } else {
        // open and close redirection paths
        if (file_paths[0]) {
            if (close(STDIN_FILENO) < 0) {
                printf("ERROR - stdin cannot close.\n");
                exit(1);
                return -1;
            } else {
                if (open(file_paths[0], O_RDONLY, 0400) < 0) {
                    printf("ERROR - redirect input path cannot open.\n");
                    exit(1);
                    return -1;
                }
            }
        }
        if (file_paths[1]) {
            if (close(STDOUT_FILENO) < 0) {
                printf("ERROR - stdout cannot close.\n");
                exit(1);
                return -1;
            } else {
                if (open(file_paths[1], O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0) {
                    printf("ERROR - redirect output path cannot open.\n");
                    exit(1);
                    return -1;
                }
            }
        }
        if (file_paths[2]) {
            int close_val = ;
            if (close(STDOUT_FILENO) < 0) {
                printf("ERROR - stdout cannot close.\n");
                exit(1);
                return -1;
            } else {
                if (open(file_paths[2], O_RDWR | O_CREAT | O_APPEND, 0666) < 0) {
                    printf("ERROR - redirect append path cannot open.\n");
                    exit(1);
                    return -1;
                }
            }
        }
        // execute the command
        if (execv(cmd, argv) == 0) {
            printf("ERROR - execute failed.\n");
            exit(1);
            return -1;
        }
    }
    wait(0);
    return 0;
}


int main() {
    // Initiate core arrays to be used for each command line.
    char buffer[BUFFER_LENGTH], *argv[MAX_ARGC], *file_paths[REDIRECT_TYPES];
    while (1) {
        // print "33sh>" for prompt-included version of shell.
        #ifdef PROMPT
            if (printf("33sh> ") < 0) {
                printf("prompt unavailable.\n");
                return -1;
            }
            fflush(stdout);
        #endif
        // read command line: string is stored in buffer.
        memset(buffer, '\0', BUFFER_LENGTH);
        ssize_t num_input = read(STDIN_FILENO, buffer, BUFFER_LENGTH);
        // error check for read.
        if (num_input < 0) {
            printf("ERROR - reading command line.\n")
            return -1;
        // case 1: exit shell if no input at all (CTRL-D)
        } else if (num_input == 0) {
            return 0;
        // case 2: new command line if enter is pressed.
        } else if (buffer[0] == '\n') {
            continue;
        // case 3 (main code): non-zero input. 
        } else {
            // possibly not needed: test! buffer[num_input - 1] = '\0';

            // reinitialize argv (command arguments) and file_paths for redirections
            memset(argv, '\0', sizeof(argv));
            memset(file_paths, '\0', sizeof(file_paths));
            // parse the command line: print error and continue if error occurs.
            if (parse(buffer, argv, file_paths) < 0) {
                continue;
            }
            // execute commands: print error and continue if error occurs.
            int cmd = execute(argv);
            if (cmd < 0) {
                continue;
            } else if (cmd == 1){ // execute() returns 1 if cmd is not one of the four.
                // handle input and output redirects: print error and continue if error occurs.
                if (redirect(argv, file_paths) < 0) {
                    continue;
                }
            }
            
        }
    }
}