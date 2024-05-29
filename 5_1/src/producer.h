#pragma once
#include <stdint.h>

struct producerArgs {
    uint8_t *flag;
    struct Queue *queue;
};


void produce(struct producerArgs *args);