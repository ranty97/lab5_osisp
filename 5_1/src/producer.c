#include "producer.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

void produce(struct producerArgs *args) {
    while (*args->flag) {
        Message_p message = (Message_p) malloc(sizeof(struct Message));
        message->type = 0;
        
        int size;

        do {
            size = rand() % 257;
        } while (size == 0);

        message->size = size;

        if (size == 256) {
            message->size = 0;
        }

        message->data = malloc(size);
        
        for (int i = 0; i < size; i++)
            message->data[i] = rand();
        
        uint16_t hash = 0;

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < 8; j+=3) {
                hash ^= ((uint16_t)message->data[i]) << j;
            }
        }

        message->hash = hash;
        int queueResult = 0;

        while ((queueResult = writeQueue(args->queue, message)) && *args->flag)
            usleep(100000);
        
        if (!*args->flag && queueResult) {
            free(message->data);
            free(message);
            return;
        }

        uint64_t reads = getTotalQueueReads(args->queue);
        uint64_t writes = getTotalQueueWrites(args->queue);

        printf("Produced a message! Hash: %04x. Queue stats: R: %lu W: %lu\n", message->hash, reads, writes);

        for (int i = 0; i < 10 && *args->flag; i++) {
            usleep(100000);
        }
    }

    pthread_exit(NULL);
}