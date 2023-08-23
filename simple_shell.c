#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_DIRS 10  /* Maximum number of directories in PATH */
#define MAX_ARGS 64

void handle_special_characters(char *str) {
    char *src = str;
    char *dest = str;

    while (*src) {
        if (*src == '\\') {
            src++;
            if (*src == '\0') {
                /* Handle backslash at the end */
                *dest++ = '\\';
            } else {
                /* Handle escaped character */
                *dest++ = *src++;
            }
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

void move_cursor(int x, int y) {
    printf("\x1b[%d;%dH", y, x);
    fflush(stdout);
}

int main() {
    char *params = NULL;
    size_t params_length = 0;
    int frames = 1;

    /* Get the value of PATH environment variable */
    char *path = getenv("PATH");
    char *dirs[MAX_DIRS];
    int dir_count = 0;

    /* Tokenize PATH to get individual directories */
    char *dir = strtok(path, ":");
    while (dir != NULL && dir_count < MAX_DIRS) {
        dirs[dir_count++] = dir;
        dir = strtok(NULL, ":");
    }

    while (frames) {
        write(STDOUT_FILENO, "MyShell> ", 9); /* Display prompt */

        if (getline(&params, &params_length, stdin) == -1) {
            if (feof(stdin)) {
                /* Handle Ctrl+D (end of file) */
                printf("\n");
                break;
            } else {
                perror("getline");
                exit(EXIT_FAILURE);
            }
        }

        /* Remove newline character */
        params[strcspn(params, "\n")] = '\0';

        if (strcmp(params, "exit") == 0) {
            frames = 0;  /* Exit the shell */
        } else {
            /* Split input into command and arguments */
            char *args[MAX_ARGS];
            int arg_count = 0;
            pid_t pid;
	    int i;

            /* Handle special characters */
            handle_special_characters(params);
            args[arg_count] = strtok(params, " ");

            while (args[arg_count] != NULL && arg_count < MAX_ARGS - 1) {
                arg_count++;
                args[arg_count] = strtok(NULL, " ");
            }
            args[arg_count] = NULL;

            /* Execute the command using execvp and searching in PATH */
            pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                /* Child process */
                /* Search for the command in PATH directories */
                for (i = 0; i < dir_count; i++) {
                    char path_to_exec[MAX_ARGS];
                    snprintf(path_to_exec, sizeof(path_to_exec), "%s/%s", dirs[i], args[0]);
                    execv(path_to_exec, args);
                }
                perror("execv");
                exit(EXIT_FAILURE);
            } else {
                /* Parent process */
                int status;
                waitpid(pid, &status, 0);
            }
        }
	move_cursor(1, 2); /* Move cursor to the next line */

    }

    free(params);  /* Free allocated memory */
    return 0;
}

