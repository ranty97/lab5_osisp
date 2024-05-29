#include "consumer.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void consume(struct consumerArgs *args) {

    while (*args->flag) {
        Message_p message;
        
        pthread_mutex_lock(&args->queue->mutex);
        while (readQueueNoMutex(args->queue, &message) && *args->flag)
            pthread_cond_wait(&args->queue->readCondition, &args->queue->mutex);
        pthread_cond_signal(&args->queue->writeCondition);
        pthread_mutex_unlock(&args->queue->mutex);

        if (!*args->flag)
            return;

        uint16_t hash = 0;

        int size = message->size;

        if (size == 0)
            size = 256;

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < 8; j+=3) {
                hash ^= ((uint16_t)message->data[i]) << j;
            }
        }

        if (message->hash != hash)
            printf("WARNING!!! HASHES DIDNT MATCH!\n");

        uint64_t reads = getTotalQueueReads(args->queue);
        uint64_t writes = getTotalQueueWrites(args->queue);

        printf("Message consumed! Desired hash: %04x; Computed hash: %04x. Queue stats: R: %lu, W: %lu \n", message->hash, hash, reads, writes);
        free(message->data);
        free(message);
        for (int i = 0; i < 10 && *args->flag; i++) {
            usleep(100000);
        }
    }

    pthread_exit(NULL);
}