#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "helpers.h"

// COPY THIS PARSE FUNCTION INTO THE "helpers.c" FILE AND DELETE THE PREVIOUS PARSE FUNCTION
// THIS VERSION SHOULD WORK PROPERLY
// MAKE SURE TO UPDATE THE "helpers.h" HEADER FILE

char ** parse(char*line,char*delim){

    char**array=malloc(sizeof(char*));
    *array=NULL;
    int n = 0;

    char*buf = strtok(line,delim);

    if (buf == NULL){
        free(array);
        array=NULL;
        return array;
    }

    while(buf!=NULL ){

        char**temp = realloc(array,(n+2)*sizeof(char*));

        if(temp==NULL){
            free(array);
            array=NULL;
            return array;
        }

        array=temp;
        temp[n++]=buf;
        temp[n]=NULL;

        buf = strtok(NULL,delim);

    }

    return array;
}

//NEW TEST MAIN THAT WORKS WITH THE PARSE FUNCTION

int main(){

    char _line[1000] = "a line of text\n";
    char * line = strdup(_line);
    char ** array = parse(line," \n");

    if (array==NULL)
        exit(1);

    int i = 0;
    while (array[i]!=NULL)
        printf("%s\n",array[i++]);

    free(array);
    free(line);

}


