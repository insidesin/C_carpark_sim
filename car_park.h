/* ---------------------------------------------------------------------------
** File: car_park.h
** Description: car_park a simulator for a car park, which has only one entry, 
** one exit and 10 parking bays. The entry allows only one car to enter the car 
** park at a time and the exit only allows a car to leave at a time. When a car 
** arrives, it randomly picks up a parking bay to park.
**
** Author: Jackson Powell - n8600571
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define CAR_PARK_SIZE 10
#define TRUE 1
#define CLOCK_PRECISION 1E9  /* one billion */

struct car_park {
	char carId[7];
	struct timespec arriveTime;
};

typedef struct car_park car;

//An array of car spaces in the car_park
car car_park[CAR_PARK_SIZE];

//Constant for an empty park.
const car EMPTY_PARK = { "\0", { -1, 0 } };


/* 
 * car_enter_park allows for the insertion of new cars into
 * the car park which is an array of type car, only if they can fit.
 * @param item is a specific car datatype to be inserted.
 */
int car_enter_park(car item);

/* 
 * car_enter_park allows for the removal of cars from the
 * car park which is an array of type car, only if there is
 * a car to remove.
 */
int car_exit_park();

/* 
 * The arrival (producer) thread that handles the introduction of new
 * cars to the car park simulator. Operates on a 0.5 sleep loop.
 */
void *arrival(void *param);

/* 
 * The departure (consumer) thread that handles the removal of existing
 * cars from the car park simulator. Operates on a 0.5 sleep loop.
 */
void *departure(void *param);

/* 
 * The monitor thread handles all input from the user and then acts 
 * accordingly to exit the program 'q'/'Q' or show the current car park
 * state 'p'/'P'.
 */
void *monitor(void *param);

/* 
 * Function that allows quick displaying of the car park state by
 * iterating through the car spaces and displaying what is currently
 * being used and by whom.
 */
void display_car_park();
