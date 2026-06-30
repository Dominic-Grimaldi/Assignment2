#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include "buffer.h"

int main(int argc, char *argv[]) {
	if (empty == SEM_FAILED || full == SEM_FAILED || mutex == SEM_FAILED) {
    		perror("sem_open failed");
    		return 1;
	}

	if (argc != 3) {
    		printf("Usage: ./consumer <id> <num_items>\n");
    		return 1;
	}

	int consumer_id = atoi(argv[1]);
	int num_items = atoi(argv[2]);

	int shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);

	if (shm_id == -1) {
    		perror("shmget failed");
    		return 1;
	}

	shared_buffer_t *buffer = (shared_buffer_t *)shmat(shm_id, NULL, 0);

	if (buffer == (void *) -1) {
    		perror("shmat failed");
    		return 1;
	}
	
	sem_t *empty = sem_open("/sem_empty", 0);
	sem_t *full  = sem_open("/sem_full",  0);
	sem_t *mutex = sem_open("/sem_mutex", 0);

	for (int i = 0; i < num_items; i++) {
		sem_wait(full);
		sem_wait(mutex);
		
		item_t item = buffer->buffer[buffer->tail];
		buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
		buffer->count--;

		printf("Consumer %d: Consumed value %d from Producer %d\n", consumer_id, item.value, item.producer_id);
		
		sem_post(mutex);
		sem_post(empty);
	}

	shmdt(buffer);
	sem_close(empty);
	sem_close(full);
	sem_close(mutex);

	return 0;
}
