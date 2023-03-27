#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 8192
#define MAXARGS 128

extern char **environ;

// Most of the code was from Textbook Bryant Ohallaron  pg 755
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void unix_error(char *msg);
char *Fgets(char *ptr, int n, FILE *stream);

int main()
{
    char cmdline[MAXLINE];
    while (1)
    {
        printf("Hello and Welcome to my Shell!");
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
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
        return;

    if (!builtin_command(argv))
    {
        if ((pid = fork()) == 0)
        {
            if (execve(argv[0], argv, environ) < 0)
            {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        if (!bg)
        {
            int status;
            if (waitpid(pid, &status, 0) < 0)
                unix_error("waitfg: waitpid error");
        }
        else
        {
            printf("%d %s", pid, cmdline);
        }
    }
    return;
}

int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "quit"))
        exit(0);
    if (!strcmp(argv[0], "help"))
    {
        printf("Type quit to exit the shell.\n");
        printf("Type program names and arguments, and hit enter.\n");
        printf("The following are built in:\n");
        printf("cd [dir] -- change the current working directory\n");
        printf("pwd -- print the current working directory\n");
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
