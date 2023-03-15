#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 100
#define MAX_PATH_LENGTH 1024

//function to exit shell
void exit_shell() {
    printf("Exiting shell...\n");
    exit(0);
}

//function that displays help with user type help in command line
void display_help() {
printf("Hello and welcome to the help page this is meant to help you work around the shell\n");
printf("Available commands:\n");
    printf("help: Displays this help message\n");
    printf("exit: Exit the shell\n");
    printf("wait: Wait for any child process to terminate\n");
    printf("pwd:Print the current working directory\n");
    printf("cd: Change the current working directory\n");
}

//blocks calling of rpcoess until on of the chiild processes terminated then returns PID of the terminated child or -1 if no child
void wait_for_child_process() {
    int status;
    //wait for child to terminated store exit status 
    pid_t pid = wait(&status);
    //print PID and exit status of terminated child 
    printf("Process %d terminated with status %d\n", pid, status);
}

void print_current_working_directory() {
    //sotre current working directory
    char path[MAX_PATH_LENGTH];
    //get working direct store it to path 
    if (getcwd(path, MAX_PATH_LENGTH) != NULL) {
        //print contents of path
        printf("%s\n", path);
        //error handling
    } else {
        printf("Error: Could not get current working directory\n");
    }
}
//changes working directory
void change_directory(char *directory) {
    if (chdir(directory) == 0) {
        printf("Changed directory to %s\n", directory);
    } else {
        printf("Error: Could not change directory to %s\n", directory);
    }
}
//takes a command and arguement and process the command with the function
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
    printf("Hello and welcome to my shell\n");
    //user input
    char input[MAX_INPUT_LENGTH];
    // the command and arguements by user
    char *command, *argument;
    // will keep going until exit 
    while (1) {

        printf(">> ");
        //reads input 
        fgets(input, MAX_INPUT_LENGTH, stdin);

        //split strings to tokens
        command = strtok(input, " \n");
        //if no more tokens asigned null
        argument = strtok(NULL, " \n");

        if (command != NULL) {
            process_command(command, argument);
        }
    }

    return 0;
}
