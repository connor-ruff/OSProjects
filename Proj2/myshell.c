#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>


int parseCLinput(char *);
int handleStart(char **, int);
int handleRun(char **, int);
int handleWait(int);
int handleMURDER(char **, int, int);

int main(int argc, char ** argv){

    // The Buffer
    char comLine [4096];

    // Continually display shell prompt
    int done = 0;
    while(done == 0){
        printf("\nmyshell> ");

        if ( fgets(comLine, sizeof(comLine), stdin) == NULL) {
            break; // EOF
        }

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

    // Check First Argument as handle as needed, send to appropriate handler
    if (!strcmp(argys[0], "start")) {
        return handleStart(argys, nwords);
    } else if (!strcmp(argys[0], "wait")){
        return handleWait(nwords);
    } else if (!strcmp(argys[0], "run")){
        return handleRun(argys, nwords);
    } else if (!strcmp(argys[0], "kill")){
        return handleMURDER(argys, nwords, SIGKILL);
    } else if (!strcmp(argys[0], "stop")){
        return handleMURDER(argys, nwords, SIGSTOP);
    } else if (!strcmp(argys[0], "continue")){
        return handleMURDER(argys, nwords, SIGCONT);
    } else {
        printf("myshell: Unknown command: %s\n", argys[0]);
        return 0;
    }
    

    return 0;
}

int handleStart(char ** argys, int nwords){
   
   // fork it
    int rc = fork();
    if ( rc < 0  ){
        printf("myshell: Failure on fork(): %s", strerror(errno));
        exit(1);
    } 
    
    // child process
    else if (rc == 0 ) { 
        printf("myshell: process %d started\n", (int) getpid());

        // prepare arguments for execvp()
        char * myargs[nwords-1];
        for (int i=0; i < nwords - 1; ++i){
            myargs[i] = strdup(argys[i+1]);
        }
        myargs[nwords - 1] = NULL;

        if ( execvp(myargs[0], myargs) == -1){
            printf("myshell: execvp(): %s: %s\n", strerror(errno), myargs[0]);
        }

       return 1;

    // the parent simply returns and continues the shell prompts
    } else {   
        return 0;
    }

}

int handleRun(char ** argys, int nwords){
   

   // Fork the process
    int rc = fork();

    // failure on fork
    if ( rc < 0  ){
        printf("myshell: Failure on fork(): %s\n", strerror(errno));
        exit(1);
    } 
    // child process if rc == 0
    else if (rc == 0 ) { 
        printf("myshell: process %d started\n", (int) getpid());

        // prepare arguments for execvp()
        char * myargs[nwords-1];
        for (int i=0; i < nwords - 1; ++i){
            myargs[i] = strdup(argys[i+1]);
        }
        myargs[nwords - 1] = NULL;

        // run execvp
        if ( execvp(myargs[0], myargs) == -1){
            printf("myshell: execvp(): %s: %s\n", strerror(errno), myargs[0]);
        }

       return 1;

    // the parent process
    } else {   

        // wait for child process
        int childRetStatus;
        int wc;
        if ((wc = waitpid(rc, &childRetStatus, 0)) == -1) {
            printf("myshell: error in waidpid(): %s\n", strerror(errno));
            exit(1);
        }
        
        // indicates child was affected by some signal
        if (WIFSIGNALED(childRetStatus)) {
            printf("myshell: process %d exited abnormally with signal %d: %s\n", wc, WTERMSIG(childRetStatus), strsignal(WTERMSIG(childRetStatus)));  
        }
        
        // child exited normally, but return code still made indicate an error!
         else if (WIFEXITED(childRetStatus)){
            printf("myshell: process %d exit normally with status %d\n", wc, WEXITSTATUS(childRetStatus));
        }

        // probably won't run...
         else{
            printf("myshell: unexpected behavior in process %d\n", wc);
        }

        return 0;
    }
}



int handleWait(int nwords){

   // check args
   if (nwords != 1){
       printf("myshell: improper number of arguments. wait is called without arguments\n");
       return 0;
   }

    int waitStatus;
    int rc;

    // run the wait MF
    if ( (rc = wait(&waitStatus)) == -1){
        printf("myshell: %s\n", strerror(errno));
    }

    // indicates child was affected by some signal
    else if (WIFSIGNALED(waitStatus)) {
        printf("myshell: process %d exited abnormally with signal %d: %s\n", rc, WTERMSIG(waitStatus), strsignal(WTERMSIG(waitStatus)));  
    }
    
    // child exited normally, but return code still made indicate an error!
    else if (WIFEXITED(waitStatus)){
        printf("myshell: process %d exit normally with status %d\n", rc, WEXITSTATUS(waitStatus));
    }

    // probably won't run...
    else{
        printf("myshell: unexpected behavior in process %d\n", rc);
    }

    return 0;
}

int handleMURDER(char ** argys, int nwords, int siggy){

    // check args
    if (nwords != 2){
        printf("myshell: improper arguments for %s. Expecting \"%s <pid>\"\n", argys[0], argys[0]);
        return 0;
    }

    // convert PID to integer (0 return implies not an integer)
    int pid;
    if ( (pid = atoi(argys[1]))  == 0 ) {
        printf("myshell: invalid pid \"%s\". Expecting \"%s <pid>\"\n", argys[1], argys[0]);
    }

    // call kill
   if ( kill(pid, siggy) == -1){
       printf("myshell: error in kill(): %s\n", strerror(errno));
       return 0;
   } 
   
   // print return message
   else {
       if (siggy == SIGKILL){
           printf("process %d killed\n", pid);
       } else if (siggy == SIGCONT){
            printf("process %d continued\n", pid);
       } else if (siggy == SIGSTOP){
            printf("process %d stopped\n", pid);
       }


   }

    return 0;
}