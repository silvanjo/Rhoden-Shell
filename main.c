#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 10

void parse_command(char* cmd, char** argv) {
    argv[0] = strtok(cmd, " ");
    for (int i = 1; i < MAX_ARGS; i++) {
        argv[i] = strtok(NULL, " ");
        if (argv[i] == NULL) {
            break;
        }
    }
}

/* Handle 'cd' command */
void change_directory(char* path) {
    if (chdir(path) != 0) {
        perror("change directory command failed");
    }
}

void exit_command() {
    exit(0);
}

int main() {
    char cmd[MAX_CMD_LEN];
    char* argv1[MAX_ARGS];
    char* argv2[MAX_ARGS];
    int has_pipe = 0;
    int fd[2];

    while (1) {
        printf("Shell> ");
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) break;

        /* Remove newline character */
        cmd[strcspn(cmd, "\n")] = 0;

        /* Check if pipe is present */
        if (strstr(cmd, "|") != NULL) {
            has_pipe = 1;
            char* first_part = strtok(cmd, "|");
            char* second_part = strtok(NULL, "|");

            parse_command(first_part, argv1);
            parse_command(second_part, argv2);
        } else {
            has_pipe = 0;
            parse_command(cmd, argv1);
        }

        if (strcmp(argv1[0], "exit") == 0) break;

        if (strcmp(argv1[0], "cd") == 0) {
            if (argv1[1] == NULL) {
                fprintf(stderr, "cd: expected argument\n");
            } else if (chdir(argv1[1]) != 0) {
                perror("cd");
            }
            continue;
        }
                
        if (has_pipe) {
            if (fork() == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(argv1[0], argv1);
                perror("execvp");
                return 1;
            }

            if (fork() == 0) {
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(argv2[0], argv2);
                perror("execvp");
                return 1;
            }

            close(fd[0]);
            close(fd[1]);

            wait(NULL);
            wait(NULL);
        } else {
            if (fork() == 0) {
                execvp(argv1[0], argv1);
                perror("execvp");
                return 1;
            }

            wait(NULL); 
        }
    }

    return 0;
}
