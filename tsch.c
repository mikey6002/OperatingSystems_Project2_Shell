#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAXLINE 8192
#define MAXARGS 128
#define MAXJOBS 10

//https://man7.org/linux/man-pages/man7/environ.7.html
extern char **environ;

// Most of the code was from Textbook Bryant Ohallaron  pg 755
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void unix_error(char *msg);
char *Fgets(char *ptr, int n, FILE *stream);


// array to store the PIDs of all the background jobs
pid_t bg_jobs[MAXJOBS];
int num_jobs = 0;

int main()
{
    printf("Hello and Welcome to my Shell!");
    char cmdline[MAXLINE];
    while (1)
    {
        printf("> ");
        fflush(stdout);

        Fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        eval(cmdline);
    }
}

void eval(char *cmdline)
{
    char *commands[MAXARGS][MAXARGS]; // array of arrays of strings
    char buf[MAXLINE];
    int bg, i, j, status;
    pid_t pid;
    int num_commands = 0;
    int pipefds[MAXARGS][2];
    //int fd_in = STDIN_FILENO, fd_out;

    strcpy(buf, cmdline);
    bg = parseline(buf, commands[0]);
    if (commands[0][0] == NULL)
        return;

    // tokenize commands on pipe
    for (i = 0; commands[i] != NULL && commands[i][0] != NULL; i++) {
        num_commands++;
        for (j = 0; commands[i][j] != NULL; j++) {
            if (strcmp(commands[i][j], "|") == 0) {
                commands[i][j] = NULL; // end current command
                commands[i+1][0] = commands[i][j+1]; // start next command
                commands[i][j+1] = NULL;
            }
        }
    }

    if(builtin_command(commands[0])){
      return;
    }

    // create the pipes
    for (i = 0; i < num_commands-1; i++)
        pipe(pipefds[i]);

    // execute the commands
    for (i = 0; i < num_commands; i++) {
        if ((pid = fork()) == 0) {
            // redirect input
            if (i == 0 && bg == 0) {
              //  fd_in = STDIN_FILENO;
            } else if (i > 0) {
                dup2(pipefds[i-1][0], STDIN_FILENO);
                close(pipefds[i-1][0]);
            }

            // redirect output
            if (i == num_commands-1 && commands[i][0] != NULL && commands[i+1][0] == NULL) {
               // fd_out = STDOUT_FILENO;
            } else if (i < num_commands-1) {
                dup2(pipefds[i][1], STDOUT_FILENO);
                close(pipefds[i][1]);
            }

            // execute command
            execvp(commands[i][0], commands[i]);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // close all pipes
    for (i = 0; i < num_commands-1; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    // wait for the last child process to finish
    if (!bg) {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
        bg_jobs[num_jobs++] = pid;
        printf("[%d] %d\n", num_jobs, pid);
    }
}

//textbook only modified to fit project 
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "quit")){
        printf("Exiting");
        exit(1);
    }
    if (!strcmp(argv[0], "wait")){
        int i;
        for (i = 0; i < num_jobs; i++)
        {
            waitpid(bg_jobs[i], NULL, 0); // wait for the background job to terminate
            printf("[%d] Done\t %d\n", i+1, bg_jobs[i]); // print the job number and PID to the console
        }
        num_jobs = 0; // reset the number of background jobs to 0
        return 1;
    }
    if (!strcmp(argv[0], "help"))
    {
        printf("Type quit to exit the shell.\n");
        printf("Type program names and arguments, and hit enter.\n");
        printf("The following are built in:\n");
        printf("cd [dir] -- change the current working directory\n");
        printf("pwd -- print the current working directory\n");
        printf("wait-- to wait for all background commands to teriminate before next command\n");
        printf("help -- display this help message\n");
        return 1;
    }
    if (!strcmp(argv[0], "pwd"))
    {
        char cwd[MAXLINE];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("getcwd() error");
        }
        return 1;
    }
    if (!strcmp(argv[0], "cd"))
    {
        if (argv[1] == NULL)
        {
            fprintf(stderr, "cd: expected argument to \"cd\"\n");
        }
        else if (chdir(argv[1]) != 0)
        {
            perror("cd error");
        }
        return 1;
    }
    if (!strcmp(argv[0], "&"))
        return 1;
    return 0;

}

int parseline(char *buf, char **argv)
{
    char *delim;
    int argc;
    int bg;

    buf[strlen(buf) - 1] = ' ';
    while (*buf && (*buf == ' '))
        buf++;

    argc = 0;
    while ((delim = strchr(buf, ' ')))
    {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' '))
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0)
        return 1;

    if ((bg = (*argv[argc - 1] == '&')) != 0)
        argv[--argc] = NULL;
        bg =1;

    return bg;
}

void unix_error(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

char *Fgets(char *ptr, int n, FILE *stream)
{
    char *rptr;
    if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
        unix_error("Fgets error");
    return (rptr);
}
