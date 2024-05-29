#pragma once
#include <stdint.h>

struct consumerArgs {
    uint8_t *flag;
    struct Queue *queue;
};


void consume(struct consumerArgs *args);