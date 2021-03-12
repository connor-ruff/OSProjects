#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
#include<math.h>
#include<time.h>

void runMandel(int, float);

int main(int argc, char** argv){

    int simProcesses;
    // Check CL Input
    if (argc == 1){
        simProcesses = 1;
    }
    else if (argc != 2) {
        printf("Improper Number of Arguments\n");
        printf("Usage: ./mandelmovie <number of processes to run>\n");
    } else {
        if( (simProcesses = atoi(argv[1])) == 0){
            printf("Invalid Argument: %s. Number of processes must be an integer greater than 0\n", argv[1]);
            printf("Usage: ./mandelmovie <number of processes to run>\n");
        }
    }

    // Set Up Variables
    int totalLeft = 50;
    float scale = 2.0;
    float scaleFactor = 1.0/1.27; // NOTE: this factor will end with desired scale of 0.000016

    // Begin First Processes
   for (int i = 0; i < simProcesses; ++i){
        runMandel(totalLeft, scale);
        totalLeft--; // Decrement Total
        scale = scale * scaleFactor; // Re-Scale
   }

    //int wc;

    // Wait for processes to complete, when one does, start another
    while (totalLeft > 0){
        // wait for completed processes (NOTE: if parent runs first, there may be nothing to wait for, hence the second condition here)
        if ( wait(NULL) == -1 && errno != ECHILD){
            printf("Error in wait() function: %s. Check that mandel is compiled\n", strerror(errno));
            exit(1);
        }
        // Run a new process when one returns
        runMandel(totalLeft, scale);
        totalLeft--;
        scale = scale * scaleFactor;
    }

    if ( wait(NULL) == -1){
        printf("Error in wait() function: %s. Check that mandel is compiled\n", strerror(errno));
        exit(1);
    }

    return 0;
}

void runMandel(int totalLeft, float scale){

    // Calculate bmp image number (1-50)
    int imageNum = 50 - totalLeft + 1;

    int rc;
    // Fork A Process
    rc = fork();
    if (rc < 0){
        printf("myshell: Failure on fork(): %s\n", strerror(errno));
        exit(1);
    }
    // Child (run execvp())
    else if (rc == 0){

        // Set up args for execvp()
        char * inArgs [16];
        inArgs[0] = "./mandel";
        inArgs[1] = "-x";
        inArgs[2] = "-0.0001";
        inArgs[3] = "-y";
        inArgs[4] = "0.8135";
        inArgs[5] = "-W";
        inArgs[6] = "1000";
        inArgs[7] = "-H";
        inArgs[8] = "1000";
        inArgs[9] = "-m";
        inArgs[10] = "1800";
        inArgs[11] = "-s";

        // Set the scale argument
        char buf[100];
        sprintf(buf, "%.8f", scale);
        inArgs[12] = strdup(buf);
       
        inArgs[13] = "-o";
        // Set the output file argument
        sprintf(buf, "mandel%d.bmp", imageNum);
        inArgs[14] = strdup(buf);

        // Run execvp with error checking   
        if ( execvp(inArgs[0], inArgs) == 0 ) {
            printf("Error in execvp(): %s\n", strerror(errno));
             exit(1);
        }
    }

    // Parent
    else {
        return; // Simply continue
    }
}