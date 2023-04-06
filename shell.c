#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAXLINE 8192
#define MAXARGS 128
#define MAXJOBS 10

//from textbook
// further reaserch from manpage https://man7.org/linux/man-pages/man7/environ.7.html
extern char **environ;

pid_t bg_jobs[MAXJOBS];
int num_jobs = 0;

void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void unix_error(char *msg);
char *Fgets(char *ptr, int n, FILE *stream);

// Most of the structure was taken from textbook bryant o'hallaron pg 755

int main()
{
    printf("Hello and Welcome to my Shell Of DOOM!\n");
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
    char *commands[MAXARGS];
    char buf[MAXLINE];
    int bg, i, status, num_cmds = 0;
    pid_t pid;

    strcpy(buf, cmdline);
    bg = parseline(buf, commands);
    if (commands[0] == NULL)
        return;

    if (builtin_command(commands))
        return;

    // parse command line for multiple commands separated by pipes
    char *cmd_ptrs[MAXARGS];
    cmd_ptrs[num_cmds++] = commands[0];
    for (i = 1; commands[i] != NULL; i++)
    {
        if (strcmp(commands[i], "|") == 0)
        {
            commands[i] = NULL;
            cmd_ptrs[num_cmds++] = commands[i+1];
        }
        else if (strcmp(commands[i], "<") == 0)
        {
            commands[i] = NULL;
            int fd_in = open(commands[i+1], O_RDONLY);
            if (fd_in < 0)
            {
                fprintf(stderr, "Error: cannot open input file %s\n", commands[i+1]);
                return;
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        else if (strcmp(commands[i], ">") == 0)
        {
        //https://www.cs.helsinki.fi/u/gurtov/c02/file_sys.htm
            commands[i] = NULL;
            int fd_out = open(commands[i+1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            if (fd_out < 0)
            {
                fprintf(stderr, "Error: cannot open output file %s\n", commands[i+1]);
                return;
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
    }

    int pipes[num_cmds - 1][2];

    for (i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            fprintf(stderr, "Pipe failed\n");
            return;
        }
    }

    int fd_in = STDIN_FILENO;

    for (i = 0; i < num_cmds; i++)
    {
        // create child process for current command
        if ((pid = fork()) == 0)
        {
            if (i < num_cmds - 1)
            {
                // connect output of current process to input of next process
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
            if (fd_in != STDIN_FILENO)
            {
                // connect input of current process to output of previous process
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            // execute command
            //https://www.ibm.com/docs/en/i/7.1?topic=functions-getenv-search-environment-variables
            char* path = getenv("PATH");
            char* p = strtok(path, ":");
            char prog[MAXARGS];
            while (p != NULL) {
                sprintf(prog, "%s/%s", p, cmd_ptrs[i]); 
                execve(prog, commands, environ);
                p = strtok(NULL, ":");
            }
            printf("%s: Command not found\n", cmd_ptrs[i]);
            exit(0);
        }

        // close pipes in parent process
        if (fd_in != STDIN_FILENO)
            close(fd_in);

        if (i < num_cmds - 1)
        {
            close(pipes[i][1]);
            fd_in = pipes[i][0];
        }

        // wait for the last child process to finish
        if (i == num_cmds - 1)
        {
            if (!bg) {
                do {
                    waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            } else {
                bg_jobs[num_jobs++] = pid;
                printf("[%d] %d\n", num_jobs, pid);
            }
        }
    }

    // close all pipes in parent process
    for (i = 0; i < num_cmds - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

// copied from pg 754 but modifed bryant o'hallaron 
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "quit")){
        printf("Exiting\n");
        exit(1);
    }
    if (!strcmp(argv[0], "cd")) {
        if (argv[1] == NULL)
            chdir(getenv("HOME"));
        else
            chdir(argv[1]);
        return 1;
    }
    if (!strcmp(argv[0], "help"))
    {
        printf("Type quit to exit the shell.\n");
        printf("Type program names and arguments, and hit enter.\n");
        printf("The following are built in:\n");
        printf("cd [dir] -- change the current working directory\n");
        printf("pwd -- print the current working directory\n");
        printf("wait-- to wait for all background commands to terminate before next command\n");
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
     if (!strcmp(argv[0], "wait"))
    {
        int i, status;
        for (i = 0; i < num_jobs; i++) {
            waitpid(bg_jobs[i], &status, WUNTRACED);
            printf("[%d] %d exited with status %d\n", i+1, bg_jobs[i], WEXITSTATUS(status));
        }
        num_jobs = 0;
        return 1;
    }
    
    
    
    return 0;
}

//mostly from textbook
int parseline(char *buf, char **argv)
{
    char *delim;
    int argc;
    int bg;
    int fd_in = STDIN_FILENO;  // default input is stdin
    int fd_out = STDOUT_FILENO;  // default output is stdout

    buf[strlen(buf) - 1] = ' ';
    while (*buf && (*buf == ' '))
        buf++;

    argc = 0;
    while ((delim = strchr(buf, ' ')))
    {
        // handle input redirection
        if (*delim == '<') {
            *delim = '\0';
            fd_in = open(buf, O_RDONLY);
            if (fd_in < 0) {
                fprintf(stderr, "%s: No such file or directory\n", buf);
                return -1;
            }
            buf = delim + 1;
            continue;
        }

        // handle output redirection
        //https://people.cs.pitt.edu/~delis/CS1550-Spring13/cs1550-recitation2.pdf
        if (*delim == '>') {
            *delim = '\0';
            fd_out = open(buf, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd_out < 0) {
                fprintf(stderr, "%s: Error opening file\n", buf);
                return -1;
            }
            buf = delim + 1;
            continue;
        }

        // handle normal arguments
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

    // set input/output file descriptors
    //https://linuxhint.com/dup2_system_call_c/
    if (fd_in != STDIN_FILENO) {
        if (dup2(fd_in, STDIN_FILENO) != STDIN_FILENO) {
            fprintf(stderr, "Error setting input redirection\n");
            return -1;
        }
        close(fd_in);
    }

    if (fd_out != STDOUT_FILENO) {
        if (dup2(fd_out, STDOUT_FILENO) != STDOUT_FILENO) {
            fprintf(stderr, "Error setting output redirection\n");
            return -1;
        }
        close(fd_out);
    }

    return bg;
}

//more from textbook
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
