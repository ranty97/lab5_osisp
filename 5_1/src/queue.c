#include "queue.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct Queue *createQueue(uint64_t capacity) {
    Message_p *buffer = malloc(capacity * sizeof(Message_p));

    if(buffer == NULL)
        return NULL;
    
    struct Queue *queue = malloc(sizeof(struct Queue));
    if (queue == NULL) {
        free(buffer);
        return NULL;
    }

    pthread_mutex_init(&queue->mutex, NULL);
    queue->head = buffer;
    queue->tail = buffer + capacity;
    queue->readCur = buffer;
    queue->writeCur = buffer;
    queue->full = 0;
    queue->size = capacity;

    return queue;
}

int readQueue(struct Queue* queue, Message_p *message) {
    pthread_mutex_lock(&queue->mutex);

	if(queue == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return 1;
    }
	
	if((queue->readCur == queue->writeCur) && !queue->full) {
        pthread_mutex_unlock(&queue->mutex);
        return 2;
    }
	
	*message = *(queue->readCur);

    Message_p *tmp = queue->readCur + 1;
	if(tmp >= queue->tail) 
        tmp = queue->head;
	if(tmp == queue->writeCur)
        queue->full = 0;
	queue->readCur = tmp;

    queue->totalRead++;
	pthread_mutex_unlock(&queue->mutex);

	return 0;
}

int writeQueue(struct Queue* queue, Message_p message) {
    pthread_mutex_lock(&queue->mutex);

    if(queue == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return 1;
    }
	
    if((queue->writeCur  == queue->readCur) && queue->full) {
        pthread_mutex_unlock(&queue->mutex);
        return 2;	
    }
	
	*(queue->writeCur) = message;
	
	Message_p *tmp = queue->writeCur + 1;
	if(tmp >= queue->tail)
        tmp = queue->head;
	if(tmp == queue->readCur)
        queue->full = 1;
	queue->writeCur = tmp;
	
    queue->totalWrite++;
    pthread_mutex_unlock(&queue->mutex);
	return 0;
}

uint64_t getTotalQueueWrites(struct Queue* queue) {
    pthread_mutex_lock(&queue->mutex);

    uint64_t totalWrites = queue->totalWrite;

    pthread_mutex_unlock(&queue->mutex);

	return totalWrites;
}

uint64_t getTotalQueueReads(struct Queue* queue) {
    pthread_mutex_lock(&queue->mutex);

    uint64_t totalReads = queue->totalRead;

    pthread_mutex_unlock(&queue->mutex);
    
	return totalReads;
}

int readQueueNoMutex(struct Queue* queue, Message_p *message) {
	if(queue == NULL) {
        return 1;
    }
	
	if((queue->readCur == queue->writeCur) && !queue->full) {
        return 2;
    }
	
	*message = *(queue->readCur);

    Message_p *tmp = queue->readCur + 1;
	if(tmp >= queue->tail) 
        tmp = queue->head;
	if(tmp == queue->writeCur)
        queue->full = 0;
	queue->readCur = tmp;

    queue->totalRead++;

	return 0;
}

void alignQueue(struct Queue* queue, uint64_t newSize) {
    uint64_t readsBefore = queue->totalRead;

    struct Queue *new = createQueue(newSize);

    Message_p message = NULL;

    while(!readQueueNoMutex(queue, &message)) {
        writeQueue(new, message);
    }

    free(queue->head);

    queue->totalRead = readsBefore;
    queue->head = new->head;
    queue->tail = new->tail;
    queue->readCur = new->readCur;
    queue->writeCur = new->writeCur;
    queue->size = new->size;
}

uint8_t increaseQueue(struct Queue* queue) {
    pthread_mutex_lock(&queue->mutex);

    alignQueue(queue, queue->size + 1);
    printf("New queue size: %ld\n", queue->size);

    pthread_mutex_unlock(&queue->mutex);
    
	return 0;
}

uint8_t decreaseQueue(struct Queue* queue) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->totalWrite - queue->totalRead < queue->size - 1) {
        alignQueue(queue, queue->size - 1);
        printf("New queue size: %ld\n", queue->size);
    }
    else {
        printf("Queue cannot be decreased!\n");
    }

    pthread_mutex_unlock(&queue->mutex);
    
	return 0;
}
