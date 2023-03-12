#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 100
#define MAX_PATH_LENGTH 1024

void exit_shell() {
    printf("Exiting shell...\n");
    exit(0);
}

void display_help() {
    printf("Available commands:\n");
    printf("help: Display this help message\n");
    printf("exit: Exit the shell\n");
    printf("wait: Wait for any child process to terminate\n");
    printf("pwd:Print the current working directory\n");
    printf("cd: Change the current working directory\n");
}

int main() {
    char input[MAX_INPUT_LENGTH];
    char *token;

    while (1) {
        printf(">> ");
        fgets(input, MAX_INPUT_LENGTH, stdin);

        // tokenize input string
        token = strtok(input, " \n");

        if (token != NULL) {
            if (strcmp(token, "help") == 0) {
                display_help();
            } else if (strcmp(token, "exit") == 0) {
                exit_shell();
            } else if (strcmp(token, "wait") == 0) {
                // wait for any child process to terminate
                int status;
                pid_t pid = wait(&status);
                printf("Process %d terminated with status %d\n", pid, status);
            } else if (strcmp(token, "pwd") == 0) {
                // print current working directory
                char path[MAX_PATH_LENGTH];
                if (getcwd(path, MAX_PATH_LENGTH) != NULL) {
                    printf("%s\n", path);
                } else {
                    printf("Error: Could not get current working directory\n");
                }
            } else if (strcmp(token, "cd") == 0) {
                // change current working directory
                token = strtok(NULL, " \n");
                if (token != NULL) {
                    if (chdir(token) == 0) {
                        printf("Changed directory to %s\n", token);
                    } else {
                        printf("Error: Could not change directory to %s\n", token);
                    }
                } else {
                    printf("Error: No directory specified\n");
                }
            } 
        }
    }

    return 0;
}
