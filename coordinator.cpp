#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

using namespace std;

#define STRING_LENGTH 256
#define MAX_STRINGS 20

void usage();
void readStrings(char*);
void allocateMessageQueue();
void allocateSharedMemory();
void deallocateMessageQueue();
void releaseSharedMemory();
void deleteSharedMemory();
void spawn();

int sCounter = 0;

key_t msgKey;
key_t shmKey;
static int msgQID;
static int shmID;

typedef struct {
	long mtype;
	char mtext[2048];
}Message;

typedef struct {
	char strings[MAX_STRINGS][STRING_LENGTH];
	pid_t pid;
}SharedMemory;

Message* msg = NULL;
SharedMemory* shmem = NULL;

int main(int argc, char* argv[]) {

	int opt;
	int c = 5;
	int m = 20;

	char * dataFile = NULL;

	while((opt = getopt(argc, argv, "hc:m:")) != -1) {
		switch(opt) {
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
			case 'c':
				if(!isdigit(*optarg) || (c = atoi(optarg)) < 0) {
					perror("coordinator.cpp: error: invalid number of total proceses");
					exit(EXIT_FAILURE);
				}
				break;

			case 'm':
				if(!isdigit(*optarg) || (m = atoi(optarg)) < 0 || (m = atoi(optarg)) > 20) {
					perror("coordinator.cpp: error: invalid number of processes");
					exit(EXIT_FAILURE);
				}
				break;
		}
	}

	if(argv[optind] == NULL) {
                perror("coordinator.cpp: error: no input file");
                exit(EXIT_FAILURE);
        }
        else {
                dataFile = argv[optind];
        }

	allocateSharedMemory();
	shmem->pid = getpid();

	//cout << "strings: " << shmem->strings[sCounter] << endl;

	readStrings(dataFile);
	releaseSharedMemory();
	allocateMessageQueue();
	
	spawn();
	
	deallocateMessageQueue();

        return EXIT_SUCCESS;
}

void usage() {
	printf("======================================================================\n");
	printf("\t\t\t\tUSAGE\n");
	printf("======================================================================\n");
	printf("coordinator -h [-c i] [-m j] dataFile\n");
	printf("run as: ./coordinator [options] dataFile\n");
	printf("======================================================================\n");
	printf("-h		:	Describe hwo to project should be run and then terminate.\n");
	printf("-c i		:	Indicate how many child processes, i, should be launched in total.\n");
	printf("-m j		:	Indicate the maximum number of children, j, allowed to exist in the system at the same time (Default 20)\n");
	printf("dataFile	:	Input file containing one string on each line\n");
}

void spawn() {

	pid_t cpid = fork();
	
	if(cpid == -1) {
		perror("coordinator.cpp: error: fork operation failed");
		exit(EXIT_FAILURE);
	}
	
	if(cpid == 0) {
	
		execl("./palin", "palin", (char*)NULL);
		exit(EXIT_SUCCESS);
	
	}

}

void readStrings(char * inputFile) {

	FILE * fp = fopen(inputFile, "r");

	char *readLine = NULL;
	size_t len = 0;
	ssize_t read;

	if(fp == NULL) {
		perror("coordinator.cpp: error: failed to open input file");
		exit(EXIT_FAILURE);
	}

	while((read = getline(&readLine, &len, fp)) != -1) {
		readLine[strlen(readLine) - 1] = '\0';
		strcpy(shmem->strings[sCounter], readLine);
		//cout << "strings: " << shmem->strings[sCounter] << endl;
		sCounter++;
	}

	fclose(fp);
	free(readLine);
	readLine = NULL;

}

void allocateSharedMemory() {

	if((shmKey = ftok("./makefile", 'p')) == -1) {
		perror("coordinator.cpp: error: shmKey ftok failed");
		exit(EXIT_FAILURE);
	}

	if((shmID = shmget(shmKey, sizeof(SharedMemory), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0) {
		perror("coordinator.cpp: error: failed to allocate shared memory");
		exit(EXIT_FAILURE);
	}
	else {
		shmem = (SharedMemory*)shmat(shmID, NULL, 0);
	}

}

void releaseSharedMemory() {
	if(shmem != NULL) {
		if(shmdt(shmem) == -1) {
			perror("coordinator.cpp: error: failed to release shared memory");
			exit(EXIT_FAILURE);
		}
	}
	deleteSharedMemory();
}

void deleteSharedMemory() {
	if(shmID > 0) {
		if(shmctl(shmID, IPC_RMID, NULL) < 0) {
			perror("coordinator.cpp: error: failed to delete shared memory"); 
			exit(EXIT_FAILURE);
		}
	}
}

void allocateMessageQueue() {

	if((msgKey = ftok("./makefile", 's')) == -1) {
		perror("coordinator.cpp: error: msgKey ftok failed");
		exit(EXIT_FAILURE);
	}

	if((msgQID = msgget(msgKey, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
		perror("coordinator.cpp: error: message queue allocation failed");
		exit(EXIT_FAILURE);
	}

}

void deallocateMessageQueue() {
	if(msgQID > 0) {
		if(msgctl(msgQID, IPC_RMID, NULL) == -1) {
			perror("coordinator.cpp: error: failed to deallocate message queue");
			exit(EXIT_FAILURE);
		}
	}
	
}


