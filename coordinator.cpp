#include <iostream>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>

using namespace std;

//macros for storing strings in shared memory
#define STRING_LENGTH 256
#define MAX_STRINGS 100

//function prototypes
void usage();
void readStrings(char*);
void allocateMessageQueue();
void allocateSharedMemory();
void deallocateMessageQueue();
void releaseSharedMemory();
void deleteSharedMemory();
void spawn(int);
void signalHandler(int);
void setTimer(int);


int sCounter = 0;			//string counter used for forking logic
int timeSeconds = 25;			//global var for timer

//key and id values for shared memory/message queue
key_t msgKey;
key_t shmKey;
int msgQID;
int shmID;

//message queue structure; source geeksforgeeks and tutorialpoint
typedef struct {
        long mtype;
        char mtext[100];
}Message;

//shared memory structure
struct SharedMemory {
        char strings[MAX_STRINGS][STRING_LENGTH];
};

//objects for shared memory and message queue
Message msg;
struct SharedMemory* shmem = NULL;

int currentProcesses = 0;				//used for logic to keep wait for processes to finish and continue
pid_t pids[MAX_STRINGS];				//issues with shared memory so i stored pids globally

string fileWrite[MAX_STRINGS];				//issues with shared memory so i globally
							//stored strings so i could write them to output files

int main(int argc, char* argv[]) {

	signal(SIGINT, signalHandler);			//signal handler for ctrl c

	//variables for options
        int opt;
        int c = 0;
        int m = 20;
        int index;

	//used for input file
        char * dataFile = NULL;

	//implementation of getopt to set options and check their validity
        while((opt = getopt(argc, argv, "hc:m:")) != -1) {
                switch(opt) {
			//displays help message
                        case 'h':
                                usage();
                                exit(EXIT_SUCCESS);
			//assigns total number of processes to c and exits if less than zero
                        case 'c':
                                if(!isdigit(*optarg) || (c = atoi(optarg)) < 0) {
                                        perror("coordinator.cpp: error: invalid number of total proceses");
                                        usage();
                                        exit(EXIT_FAILURE);
                                }
                                break;
			//assigns num of concurrent processes to m and exits if less than 0 and > 20
                        case 'm':
                                if(!isdigit(*optarg) || (m = atoi(optarg)) < 0 || (m = atoi(optarg)) > 20) {
                                        perror("coordinator.cpp: error: invalid number of concurrent processes");
                                        usage();
                                        exit(EXIT_FAILURE);
                                }
                                break;
                }
        }
        
        //was not sure what you wanted c to be as default value so i set it as 20 (default of m);
        if (c == 0) {
        	c = m;
        }
        
	//exits if no input file is supplied otherwise assigns to var
        if(argv[optind] == NULL) {
                perror("coordinator.cpp: error: no input file");
                exit(EXIT_FAILURE);
        }
        else {
                dataFile = argv[optind];
        }

	//allocates shared memory
	allocateSharedMemory();
	cout << "coordinator.cpp : shared memory has been allocated\n";
        
	//allocates message queue
	allocateMessageQueue();
	cout << "coordinator.cpp : message queue has been allocated\n";
        
	//initializes timer; used from previous project (periodicasterisk.c in book)
	setTimer(timeSeconds);
	//sleep(30);									//used for debugging
	
	cout << "coordinator.cpp : started reading strings into shared memory\n";
	readStrings(dataFile);								//reads strings into shared memory
	cout << "coordinator.cpp : finished reading strings into shared memory\n";
        
	//used for for forking logic
	index = 0;
	int pCounter = 0;
        
	//checks to make sure that number of concurrent processes is not passed and 
	//that it doesnt execute more times than the amount of strings available
        for(index = 0; index < m && index < sCounter; index++) {
        	//makes sure that it does not execute more than total number of processes allowed
		if(index < c) {
        		
			//had issues with accessing shared memory in this loop so i used globally stored
			//strings to write them to file
			int n = fileWrite[index].length();
			char outputString[n + 1];
			strcpy(outputString, fileWrite[index].c_str());
        	
        		//responsible for creating children
			spawn(index);
        	
			//receives message from child
        		msgrcv(msgQID, &msg, sizeof(Message), 0, 0);
			
			//checks to see if string is palin or not a palin and writes to appropriate file
			if(strcmp(msg.mtext, "is palin") == 0) {
				FILE * fp1 = fopen("palin.out", "a");
				if(fp1 == NULL) {
					perror("coordinator.cpp: error: failed to palin.out file");
					exit(EXIT_FAILURE);		
				}
				fprintf(fp1, "PID: %d \t Index: %d \t %s is a palindrome\n", pids[index], index, outputString);
				fclose(fp1);
		
			}
			else if (strcmp(msg.mtext, "not palin") == 0) {
				FILE * fp2 = fopen("nopalin.out", "a");
				if(fp2 == NULL) {
					perror("coordinator.cpp: error: failed to palin.out file");
					exit(EXIT_FAILURE);		
				}
				fprintf(fp2, "PID: %d \t Index: %d \t %s is not a palindrome\n", pids[index], index, outputString);
				fclose(fp2);
			}
			pCounter++;
        	}
        }
	
	while(currentProcesses > 0) {	
		
		//waits for child process to finish and decrements
		wait(NULL);
		--currentProcesses;
		
		if(pCounter == c) {
			printf("coordinator.cpp : maximum amount of processes has been reached\n");
			break;
		}

		//logic for making sure that stays in the bounds of c (total processes) and total number of processes
		//is less than the amount of strings and current processes is less than option m
		if((currentProcesses < m) && (pCounter < c) && (pCounter < sCounter)) {
		
			//same as above for loop
			int n = fileWrite[pCounter].length();
			char outputString[n + 1];
			strcpy(outputString, fileWrite[pCounter].c_str());
		
			//same as above for loop
			spawn(pCounter);
			
			msgrcv(msgQID, &msg, sizeof(Message), 0, 0);
			
			if(strcmp(msg.mtext, "is palin") == 0) {
				FILE * fp1 = fopen("palin.out", "a");
				if(fp1 == NULL) {
					perror("coordinator.cpp: error: failed to palin.out file");
					exit(EXIT_FAILURE);		
				}
				fprintf(fp1, "PID: %d \t Index: %d \t %s is a palindrome\n", pids[pCounter], pCounter, outputString);
				fclose(fp1);
		
			}
			else if (strcmp(msg.mtext, "not palin") == 0) {
				FILE * fp2 = fopen("nopalin.out", "a");
				if(fp2 == NULL) {
					perror("coordinator.cpp: error: failed to palin.out file");
					exit(EXIT_FAILURE);		
				}
				fprintf(fp2, "PID: %d \t Index: %d \t %s is not a palindrome\n", pids[pCounter], pCounter, outputString);
				fclose(fp2);
			}
			pCounter++;
		}
	}

	sleep(2);

	//deallocates and deletes shared memory and message queue
        //releaseSharedMemory();
        deleteSharedMemory();
        deallocateMessageQueue();

        return EXIT_SUCCESS;
}

//help menu to show how to run 
void usage() {
        printf("======================================================================\n");
        printf("\t\t\t\tUSAGE\n");
        printf("======================================================================\n");
        printf("coordinator -h [-c i] [-m j] dataFile\n");
        printf("run as: ./coordinator [options] dataFile\n");
        printf("if [-c i] is not defined it is set as default 20\n");
        printf("======================================================================\n");
        printf("-h              :       Describe hwo to project should be run and then terminate.\n");
        printf("-c i            :       Indicate how many child processes, i, should be launched in total.\n");
        printf("-m j            :       Indicate the maximum number of children, j, allowed to exist in the system at the same time (Default 20)\n");
        printf("dataFile        :       Input file containing one string on each line\n");
}

//responsible for forking children and acquiring child pids since i was having issues with shared memory
void spawn(int indices) {
	++currentProcesses;

        if((pids[indices] = fork()) == 0) {
        
		int length = snprintf(NULL, 0, "%d", indices);
		char* xx = (char*)malloc(length + 1);
		snprintf(xx, length + 1, "%d", indices);
        
		execl("./palin", xx, (char*)NULL);
	
		free(xx);
		xx = NULL;
		
		exit(EXIT_SUCCESS);
        }
}

//reads strings from file and removes \n and stores in shared memory
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
                //cout << "coord: strings: " << shmem->strings[sCounter] << endl;			//used for debugging
                fileWrite[sCounter] = readLine;
                sCounter++;
                //cout << sCounter << endl;								//used for debugging
        }

        fclose(fp);
        free(readLine);
        readLine = NULL;

}

//allocates shared memory and checks for failures
void allocateSharedMemory() {

        if((shmKey = ftok("./makefile", 'p')) == -1) {
                perror("coordinator.cpp: error: shmKey ftok failed");
                exit(EXIT_FAILURE);
        }

	shmID = shmget(shmKey, sizeof(struct SharedMemory), 0666 | IPC_CREAT | IPC_EXCL);

        if(shmID < 0) {
                perror("coordinator.cpp: error: failed to allocate shared memory");
                exit(EXIT_FAILURE);
        }
        else {
                shmem = (struct SharedMemory*)shmat(shmID, NULL, 0);
        }

}

//releases shared memory and deletes it; checks for failures
void releaseSharedMemory() {
	if(shmem != NULL) {
		if(shmdt(shmem) == -1) {        	
			perror("coordinator.cpp: error: failed to release shared memory");
			exit(EXIT_FAILURE);
		}
        }
	deleteSharedMemory();
}

//called in releaseSharedMemory() to delete shared memory; checks for failure
void deleteSharedMemory() {
        if(shmID > 0) {
                if(shmctl(shmID, IPC_RMID, NULL) < 0) {
                        perror("coordinator.cpp: error: failed to delete shared memory"); 
                        exit(EXIT_FAILURE);
                }
        }
}

//allocates message queue and checks for failure; source was geeksforgeeks and tutorial point
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

//deallocates and deletes message queue and source was geeksforgeeks and tutorial point
void deallocateMessageQueue() {
        if(msgQID > 0) {
                if(msgctl(msgQID, IPC_RMID, NULL) == -1) {
                        perror("coordinator.cpp: error: failed to deallocate message queue");
                        exit(EXIT_FAILURE);
                }
        }

}

//signal handler for ctrl + c and timer; kills processes; and releases/deletes shmem and msg q
//used from previous project; geeksforgeeks was source
void signalHandler(int signal) {

	if(signal == SIGINT) {
		printf("coordinator.cpp : terminating program: ctrl + c signalHandler\n");
	}
	else if (signal == SIGALRM) {
		printf("coordinator.cpp : terminating program: timer signalHandler\n");
	}
	
	for(int i = 0; i < sCounter; i++) {
		kill(pids[i], SIGKILL);
	}
	
	while(wait(NULL) > 0);
	
	releaseSharedMemory();
	deallocateMessageQueue();

	exit(EXIT_SUCCESS);

}

//sets up the timer for 25 seconds; checks for failure; set to signalHandler function
//used from previous project; source was book (periodicasterisk.c)
void setTimer(int seconds) {

	struct sigaction act;
	act.sa_handler = &signalHandler;
	act.sa_flags = SA_RESTART;
	
	if(sigaction(SIGALRM, &act, NULL) == -1) {
		perror("coordinator.cpp: error: failed to set up sigaction handler for setTimer()");
		exit(EXIT_FAILURE);
	}
	
	struct itimerval value;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 0;
	
	value.it_value.tv_sec = seconds;
	value.it_value.tv_usec = 0;
	
	if(setitimer(ITIMER_REAL, &value, NULL)) {
		perror("coordinator.cpp: error: failed to the timer in setTimer()");
		exit(EXIT_FAILURE);
	}

}

