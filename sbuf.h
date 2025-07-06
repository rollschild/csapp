#ifndef INCLUDED_SBUF_H
#define INCLUDED_SBUF_H

#include <semaphore.h>

typedef struct {
  int *buf;    /* buffer array */
  int n;       /* max number of slots */
  int front;   /* buf[(front + 1) % n] is the first item */
  int rear;    /* buf[read % n] is the last item */
  sem_t mutex; /* protects accesses to buf */
  sem_t slots; /* counts available slots */
  sem_t items; /* counts available items */
} sbuf_t;

#endif
