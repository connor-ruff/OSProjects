#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int usage(int);
void copy_message();

int main(int argc, char ** argv) {


	// Set up constants
	char * sourceFile ;
	char * targetFile;

	// Create alarm system
	signal(SIGALRM, copy_message);
	alarm(1);

	// Handle CL Arguments
	if (argc != 3) {
		printf("copyit: Improper Number of Arguments\n");
		usage(1);
	}
	else {
		sourceFile = argv[1];
		targetFile = argv[2];
	}

	// Open Source File
	int sourceFD;
	if(  (sourceFD = open(sourceFile, O_RDONLY) ) < 0 ) {
		printf("Error Opening %s:  %s\n", sourceFile, strerror(errno));
		exit(1);
	}
	// Create Target File
	int targetFD;
	if (  (targetFD = open(targetFile, O_WRONLY | O_CREAT | O_TRUNC, 644)) < 0 ){
		printf("Error Creating %s:   %s\n", targetFile, strerror(errno));
		exit(1);
	}

	// Set Up Byte Trackers
	char buf [BUFSIZ];
	int bytesTotal = 0;
	int bytesWritten;
	int bytR;
	int bytW;
	while ( 1 == 1 ){
	
		// Read Some Bytes
		if(  (bytR = read(sourceFD, buf, sizeof(buf)-1)) < 0 ) {
			if (errno == EINTR){ continue; }
			printf("Error Reading %s:   %s", sourceFile, strerror(errno));
		} else if (bytR == 0) {
			// File is done
			break;
		}

		bytesWritten = 0;
		// Write Some Bytes
		while(bytesWritten < bytR) {
			if ( (bytW = write(targetFD, buf, bytR)) <0    ) {
					if (errno == EINTR){ continue; }
				printf("Error Writing to %s:   %s", targetFile, strerror(errno));
			}
			bytesWritten = bytesWritten + bytW ;
		}
		bytesTotal = bytesTotal + bytesWritten;

	}

	// Clean up
	printf("copyit: copied %d bytes from file %s to file %s\n", bytesTotal, sourceFile, targetFile);
	
	if (  close(targetFD)  < 0  ) {
		printf("Error Closing file: %s    %s", targetFile, strerror(errno));
		exit(1);
	}
	if (close(sourceFD) < 0 ) {
		printf("Error Closing file: %s  %s", sourceFile, strerror(errno));
		exit(1);
	}

	return 0;
}

void copy_message(){
	printf("copit: still copying...\n");
	alarm(1);
}

int usage(int rt){
	printf("Usage: ./copyit <source file> <targetfile>\n");
	exit(rt);
}
