#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "helpers.h"

int parse(char* line, char*** _args, char* delim) {
    char **args = malloc(sizeof(char *));
    if (args == NULL) {
        perror("malloc");
        return -1;
    }

    int argc = 0;
    char *token = strtok(line, delim);
    while (token != NULL) {
        args[argc++] = token;
        char **tmp = realloc(args, sizeof(char *) * (argc + 1));
        if (tmp == NULL) {
            perror("realloc");
            return -1;
        }
        args = tmp;
        token = strtok(NULL, delim);
    }
    args[argc] = NULL;

    *_args = args;

    return argc;
}

int find_special(char **args, char *special) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], special) == 0) {
            return i;
        }
        i++;
    }
    return -1;
}

FILE *getInput(int argc, char* argv[]) {
    if (argc == 1) {
        // Read from stdin
        return stdin;
    } else if (argc == 2) {
        // Read from file
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) {
            perror("fopen");
            return NULL;
        }
        return fp;
    } else {
        printf("Usage: ./tsch [filename]\n");
        return NULL;
    }
}