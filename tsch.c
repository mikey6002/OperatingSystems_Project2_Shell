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

void wait_for_child_process() {
    int status;
    pid_t pid = wait(&status);
    printf("Process %d terminated with status %d\n", pid, status);
}

void print_current_working_directory() {
    char path[MAX_PATH_LENGTH];
    if (getcwd(path, MAX_PATH_LENGTH) != NULL) {
        printf("%s\n", path);
    } else {
        printf("Error: Could not get current working directory\n");
    }
}

void change_directory(char *directory) {
    if (chdir(directory) == 0) {
        printf("Changed directory to %s\n", directory);
    } else {
        printf("Error: Could not change directory to %s\n", directory);
    }
}

void process_command(char *command, char *argument) {
    if (strcmp(command, "help") == 0) {
        display_help();
    } else if (strcmp(command, "exit") == 0) {
        exit_shell();
    } else if (strcmp(command, "wait") == 0) {
        wait_for_child_process();
    } else if (strcmp(command, "pwd") == 0) {
        print_current_working_directory();
    } else if (strcmp(command, "cd") == 0) {
        change_directory(argument);
    } else {
        printf("Error: Unknown command\n");
    }
}

int main() {
    char input[MAX_INPUT_LENGTH];
    char *command, *argument;

    while (1) {
        printf(">> ");
        fgets(input, MAX_INPUT_LENGTH, stdin);

        // tokenize input string
        command = strtok(input, " \n");
        argument = strtok(NULL, " \n");

        if (command != NULL) {
            process_command(command, argument);
        }
    }

    return 0;
}
