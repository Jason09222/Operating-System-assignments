#include "opt-synchprobs.h"
#include "kitchen.h"
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>




/*
 * ********************************************************************
 * INSERT ANY GLOBAL VARIABLES YOU REQUIRE HERE
 * ********************************************************************
 */
struct cv *full;
struct cv *empty; // To perfrom syncronization
struct lock *lock; // To ensure mutual exclusion
static volatile int the_counter; // represent the number of serves remain
static volatile int served; // represent the serves the customer consumes
/*
 * initialise_kitchen: 
 *
 * This function is called during the initialisation phase of the
 * kitchen, i.e.before any threads are created.
 *
 * Initialise any global variables or create any synchronisation
 * primitives here.
 * 
 * The function returns 0 on success or non-zero on failure.
 */

int initialise_kitchen()
{
        the_counter = 0;
        served = POTSIZE_IN_SERVES;
        lock = lock_create("lock");
        if (lock == NULL) {
            return ENOMEM;
        }

        full = cv_create("full");
        empty = cv_create("empty");
        if (full == NULL || empty == NULL) {
            return ENOMEM;
        }

        return 0;
}

/*
 * cleanup_kitchen:
 *
 * This function is called after the dining threads and cook thread
 * have exited the system. You should deallocated any memory allocated
 * by the initialisation phase (e.g. destroy any synchronisation
 * primitives).
 */

void cleanup_kitchen()
{
        lock_destroy(lock); // clean-up declared cv and lock
        cv_destroy(full);
        cv_destroy(empty);
}


/*
 * do_cooking:
 *
 * This function is called repeatedly by the cook thread to provide
 * enough soup to dining customers. It creates soup by calling
 * cook_soup_in_pot().
 *
 * It should wait until the pot is empty before calling
 * cook_soup_in_pot().
 *
 * It should wake any dining threads waiting for more soup.
 */

void do_cooking()
{
        lock_acquire(lock); // mutual exclusion
        while(the_counter == POTSIZE_IN_SERVES || served != POTSIZE_IN_SERVES) { // situation that don't need to produce, then just sleep
                cv_wait(full, lock);
        }
        cook_soup_in_pot(); // if no remaining, cook 
        the_counter = the_counter + POTSIZE_IN_SERVES; // now update counter
        served = 0; // default
        cv_broadcast(empty, lock); // wake up consumer
        lock_release(lock);
}

/*
 * fill_bowl:
 *
 * This function is called repeatedly by dining threads to obtain soup
 * to satify their hunger. Dining threads fill their bowl by calling
 * get_serving_from_pot().
 *
 * It should wait until there is soup in the pot before calling
 * get_serving_from_pot().
 *
 * get_serving_from_pot() should be called mutually exclusively as
 * only one thread can fill their bowl at a time.
 *
 * fill_bowl should wake the cooking thread if there is no soup left
 * in the pot.
 */

void fill_bowl()
{
        lock_acquire(lock);
        while (the_counter == 0) { // when pot is empty, then sleep consumer and wake up producer
                cv_signal(full, lock); // wake up producerr
                cv_wait(empty, lock); // block consumer and release the lock
                // cv_signal(full, lock);
        }
        get_serving_from_pot(); // if pot not empty, get food from pot
        the_counter = the_counter - 1; // update served and the_counter
        served = served + 1;
        // cv_signal(full, lock);
        lock_release(lock);
}
