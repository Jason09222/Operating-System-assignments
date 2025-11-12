/* This file will contain your solution. Modify it as you wish. */
#include <types.h>
#include <lib.h>
#include <synch.h>
#include "producerconsumer.h"

/* Declare any variables you need here to keep track of and
   synchronise your bounded buffer. A sample declaration of a buffer is shown
   below. It is an array of pointers to items.
   
   You can change this if you choose another implementation. 
   However, your implementation should accept at least BUFFER_SIZE 
   prior to blocking
*/

#define BUFFLEN (BUFFER_SIZE + 1)

data_item_t * item_buffer[BUFFER_SIZE+1];



volatile int head, tail;
struct semaphore *consumer; // Use these two semaphore to wake and block producer and consumer
struct semaphore *producer;
struct semaphore *mutex; // Use mutex to perform mutual exclusion 
/* consumer_receive() is called by a consumer to request more data. It
   should block on a sync primitive if no data is available in your
   buffer. It should not busy wait! */

data_item_t * consumer_receive(void)
{
        data_item_t * item;

        P(consumer); // To perform the synchronisation primitives
        P(mutex); // Mutual exclusion

        while(head == tail) {
                /* busy wait */
        }
        item = item_buffer[tail];
        tail = (tail + 1) % BUFFLEN;

        V(mutex);
        V(producer);
        /******************
         * Remove above here
         */

        return item;
}

/* procucer_send() is called by a producer to store data in your
   bounded buffer.  It should block on a sync primitive if no space is
   available in your buffer. It should not busy wait!*/

void producer_send(data_item_t *item)
{
        P(producer);
        P(mutex);

        while((head + 1) % BUFFLEN == tail) {
                /* busy wait */
        }
        item_buffer[head] = item;
        head = (head + 1) % BUFFLEN;

        V(mutex);
        V(consumer);
}




/* Perform any initialisation (e.g. of global data) you need
   here. Note: You can panic if any allocation fails during setup */

void producerconsumer_startup(void)
{
        head = tail = 0;
        producer = sem_create("producer", BUFFER_SIZE); // Create the semaphore
        consumer = sem_create("consumer", 0);
        mutex = sem_create("mutex", 1);

        if (producer == NULL || consumer == NULL || mutex == NULL) {
                panic("SomeThing Goes Wrong");
        }
}

/* Perform any clean-up you need here */
void producerconsumer_shutdown(void)
{
        sem_destroy(producer); // clean-up declared semaphore
        sem_destroy(consumer);
        sem_destroy(mutex);
}

