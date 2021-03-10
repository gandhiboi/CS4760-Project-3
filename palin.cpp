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

struct SharedMemory {
	char strings[MAX_STRINGS][STRING_LENGTH];
	pid_t pid;
};

Message* msg;
struct SharedMemory* shmem = NULL;

int main(int argc, char* argv[]) {

	allocateSharedMemory();
	//allocateMessageQueue();
	
	cout << "palin.cpp: shmKey: " << shmKey << endl;
	cout << "sizeof SharedMemory Struct: " << sizeof(struct SharedMemory) << endl;
	cout << "palin: pid: " << shmem->pid << endl;
	cout << "palin: shmID: " << shmID << endl;
	

        return EXIT_SUCCESS;
}

void allocateSharedMemory() {

	if((shmKey = ftok("./makefile", 'p')) == -1) {
		perror("coordinator.cpp: error: shmKey ftok failed");
		exit(EXIT_FAILURE);
	}

	if((shmID = shmget(shmKey, sizeof(struct SharedMemory), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0) {
		perror("coordinator.cpp: error: failed to allocate shared memory");
		exit(EXIT_FAILURE);
	}
	else {
		shmem = (struct SharedMemory*)shmat(shmID, NULL, 0);
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



