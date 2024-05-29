#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdint.h>
#include "queue.h"
#include "producer.h"
#include "consumer.h"
#include <string.h>
#include <termios.h>
#include <sys/mman.h>

pthread_t *producers = NULL;
uint8_t **producerFlags = NULL;
size_t producersSize = 0;

pthread_t *consumers = NULL;
uint8_t **consumerFlags = NULL;
size_t consumersSize = 0;

char getch()
{
    struct termios new_settings, stored_settings;
   
    tcgetattr(0,&stored_settings);
   
    new_settings = stored_settings;
     
    new_settings.c_lflag &= ~(ICANON | ECHO);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    
    tcsetattr(0, TCSANOW, &new_settings);
    char result = getchar();
    tcsetattr(0, TCSANOW, &stored_settings);
    return result;
}


void createProducer(struct Queue *queue) {
    producers = realloc(producers, (++producersSize) * sizeof(pthread_t));
    producerFlags = realloc(producerFlags, producersSize * sizeof(uint8_t*));

    producerFlags[producersSize - 1] = malloc(1);
    *producerFlags[producersSize - 1] = 1;

    struct producerArgs *args = malloc(sizeof(struct producerArgs));

    args->flag = producerFlags[producersSize - 1];
    args->queue = queue;

    pthread_create(&producers[producersSize - 1], NULL, &produce, args);

    printf("Created new producer. Total producers: %ld\n", producersSize);
}

void killProducer() {
    if (producersSize == 0)
        return;
    
    producersSize--;
    *producerFlags[producersSize] = 0;
    
    pthread_join(producers[producersSize], NULL);
    free(producerFlags[producersSize]);

    if (producersSize) {
        producers = realloc(producers, producersSize * sizeof(pthread_t));
        producerFlags = realloc(producerFlags, producersSize * sizeof(uint8_t*));
    }
    else {
        free(producers);
        producers = NULL;
    }

    printf("Killed a producer. Total producers: %ld\n", producersSize);
}

void createConsumer(struct Queue *queue) {
    consumers = realloc(consumers, (++consumersSize) * sizeof(pthread_t));
    consumerFlags = realloc(consumerFlags, consumersSize * sizeof(uint8_t*));

    struct consumerArgs *args = malloc(sizeof(struct consumerArgs));

    consumerFlags[consumersSize - 1] = malloc(1);
    *consumerFlags[consumersSize - 1] = 1;

    args->flag = consumerFlags[consumersSize - 1];
    args->queue = queue;

    pthread_create(&consumers[consumersSize - 1], NULL, &consume, args);

    printf("Created new consumer. Total consumers: %ld\n", consumersSize);
}

void killConsumer() {
    if (consumersSize == 0)
        return;
    
    consumersSize--;
    *consumerFlags[consumersSize] = 0;
    
    pthread_join(consumers[consumersSize], NULL);
    free(consumerFlags[consumersSize]);

    if (consumersSize) {
        consumers = realloc(consumers, consumersSize * sizeof(pthread_t));
        consumerFlags = realloc(consumerFlags, consumersSize * sizeof(uint8_t*));
    }
    else {
        free(consumers);
        consumers = NULL;
    }

    printf("Killed a consumer. Total consumers: %ld\n", consumersSize);
}

int main() {
    struct Queue *queue = createQueue(100);
    char action;

    while ((action = getch()) != 'q') {
        switch (action) {
            case 'o': createConsumer(queue); break;
            case 'i': killConsumer(); break;
            case 'l': createProducer(queue); break;
            case 'k': killProducer(); break;
            case '+': increaseQueue(queue); break;
            case '-': decreaseQueue(queue); break;
        }
    }

    while (consumersSize)
        killConsumer();
    
    while (producersSize)
        killProducer();

    return 0;
}