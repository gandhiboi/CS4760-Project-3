#include <iostream>
#include <ctype.h>
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
#include <signal.h>
#include <wait.h>

using namespace std;

//same as coordinator.cpp; macros used for strings in shared memory
#define STRING_LENGTH 256
#define MAX_STRINGS 20

//functino prototypes
void allocateMessageQueue();
void allocateSharedMemory();
void signalHandler(int);

//keys and ids for shared memory and message queue
key_t msgKey;
key_t shmKey;
static int msgQID;
static int shmID;

//message queue structure; source was geeksforgeeks and tutorialpoint
typedef struct {
        long mtype;
        char mtext[100];
}Message;

//shared memory structure for strings in file
struct SharedMemory {
        char strings[MAX_STRINGS][STRING_LENGTH];
};

//objects for message queue and shared memory
Message msg;
struct SharedMemory* shmem = NULL;

int main(int argc, char* argv[]) {

	allocateSharedMemory();
	allocateMessageQueue();

	//signal handler for ctrl + c; same as master
	signal(SIGINT, signalHandler);

	//child sleeps for random amount of 0-3 seconds
	sleep(rand() % 3);

	//logical index for string
	int index = atoi(argv[0]);
	
	//mainly used for debugging but kept to have some sort of terminal output
	printf("PID : %d started testing string\n", getpid());

	//type for the message queue; wasn't necessary but kept just in case
	msg.mtype = 1;
	
	//takes the string from shared memory at the logical index passed to palin.cpp
	char * testString = shmem->strings[index];
	
	//used for testing if it is a palindrome
	int size = strlen(testString);
	bool flag = true;
	
	//loops through each character and tests if palindrome
	for(int i = 0; i < size; i++) {
		if(tolower(testString[i]) != tolower(testString[size - i - 1])) {
			flag = false;
			break;
		}
	} 
	
	//used to send message to master regarding if string is a palindrome or not
	if(flag == true) {
		strcpy(msg.mtext, "is palin");
		msgsnd(msgQID, &msg, sizeof(Message), 0);
	}
	else {
		strcpy(msg.mtext, "not palin");
		msgsnd(msgQID, &msg, sizeof(Message), 0);
	}
	
	//used for debugging but kept to have terminal output
	printf("PID : %d finished testing string\n", getpid());

	return EXIT_SUCCESS;
}

//allocates and attaches to shared memory
void allocateSharedMemory() {
	if((shmKey = ftok("./makefile", 'p')) == -1) {
		perror("coordinator.cpp: error: shmKey ftok failed");
		exit(EXIT_FAILURE);
        }

        if((shmID = shmget(shmKey, sizeof(struct SharedMemory), 0666)) < 0) {
                perror("coordinator.cpp: error: failed to allocate shared memory");
                exit(EXIT_FAILURE);
        }
        else {
                shmem = (struct SharedMemory*)shmat(shmID, NULL, 0);
        }
}

//allocates and attaches to message queue
void allocateMessageQueue() {
        if((msgKey = ftok("./makefile", 's')) == -1) {
                perror("palin.cpp: error: msgKey ftok failed");
                exit(EXIT_FAILURE);
        }

	if((msgQID = msgget(msgKey, S_IRUSR | S_IWUSR | IPC_CREAT)) == -1) {
        	cout << "palin: msgQID: " << msgQID << endl;
		perror("palin.cpp: error: message queue allocation failed");
		exit(EXIT_FAILURE);
        }

}

//signal handler for ctrl + c
void signalHandler(int signal) {

	if(signal == SIGINT) {
		printf("palin.cpp : terminating program: ctrl + c signalHandler\n");
	}

	exit(EXIT_FAILURE);
}

