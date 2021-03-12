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
#include <signal.h>
#include <wait.h>

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
        char mtext[100];
}Message;

struct SharedMemory {
        char strings[MAX_STRINGS][STRING_LENGTH];
        pid_t pid;
};

Message msg;
struct SharedMemory* shmem = NULL;

int main(int argc, char* argv[]) {

	allocateSharedMemory();
	allocateMessageQueue();

	cout << "palin: msgQID: " << msgQID << endl;
	cout << "palin: pid: " << shmem->pid << endl;


	msg.mtype = 1;
	cout << "palin: mtype: " << msg.mtype << endl;
	
	strcpy(msg.mtext, "palin: not a palin");
	msgsnd(msgQID, &msg, sizeof(Message), 0);

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
                perror("palin.cpp: error: msgKey ftok failed");
                exit(EXIT_FAILURE);
        }

	if((msgQID = msgget(msgKey, S_IRUSR | S_IWUSR | IPC_CREAT)) == -1) {
        	cout << "palin: msgQID: " << msgQID << endl;
		perror("palin.cpp: error: message queue allocation failed");
		exit(EXIT_FAILURE);
        }

}

