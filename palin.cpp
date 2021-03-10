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

void allocateMessageQueue();
void allocateSharedMemory();

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

Message* msg;
SharedMemory* shmem = NULL;

int main(int argc, char* argv[]) {

	allocateSharedMemory();
	//allocateMessageQueue();

	//for(int i = 0; i < MAX_STRINGS; i++) {
		cout << "palin.cpp: strings: " << shmem->strings[0] << endl;
	//}

        return EXIT_SUCCESS;
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



