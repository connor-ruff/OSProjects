#include<stdio.h>
#include<string.h>

int parseCLinput(char *);

int main(int argc, char ** argv){

    // The Buffer
    char comLine [4096];

    // Continually display shell prompt
    int done = 0;
    while(done == 0){
        printf("myshell> ");
        fgets(comLine, sizeof(comLine), stdin);

        // Parse the user's input, and direct as appropriate
        done = parseCLinput(comLine);
    }

    printf("\n");
    return 0;
}

int parseCLinput(char * comLine){

    char * argys [100];

    // Get First Argument, and Check that it is not empty, quit, or return
    argys[0] = strtok(comLine, " \t\n");
    if( argys[0]  == NULL ) { return 0; }
    else if ( !strcmp(argys[0], "exit") || !strcmp(argys[0], "quit") ) { return 1;}

    // Store the rest of the arguments
    int nwords;
    for (int i=1; i < 100; ++i){
        argys[i] = strtok(0, " \t\n");

        // If NULL, end of CL args, indicate with 0, store number of args
        if (argys[i] == NULL) {
            argys[i] = 0;
            nwords = i;
            break;
        }

    }

    // Check First Argument as handle as needed
    

    return 0;
}