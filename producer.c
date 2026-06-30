#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>


int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Usage: ./producer <id> <num_items>\n");
		return 1;
	}
	
	int producer_id = atoi(argv[1]);
	int num_items = atoi(argv[2]);

	int shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | 0666);
	shared_buffer_t *buffer = (shared_buffer_t *)shmat(shm_id, NULL, 0);

	if (buffer->count == 0) {
    		buffer->head = 0;
    		buffer->tail = 0;
    		buffer->count = 0;
	}

	sem_t *empty = sem_open("/sem_empty", O_CREAT, 0644, BUFFER_SIZE);
	sem_t *full = sem_open("/sem_full", O_CREAT, 0644, 0);
	sem_t *mutex = sem_open("/sem_mutex", O_CREAT, 0644, 1);

	for (int i = 0; i < num_items; i++) {
		sem_wait(empty);
		sem_wait(mutex);

		item_t item;
		item.value = producer_id * 1000 + i;
		item.producer_id = producer_id;

		buffer->buffer[buffer->head] = item;
		buffer->head = (buffer->head + 1) % BUFFER_SIZE;
		buffer->count++;

		printf("Producer %d: Produced value %d\n", producer_id, item.value);
		sem_post(mutex);
		sem_post(full);
	}
	shmdt(buffer);
	sem_close(empty);
	sem_close(full);
	sem_close(mutex);

	return 0;
}
