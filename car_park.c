/* ---------------------------------------------------------------------------
** File: car_park.c
** Description: car_park a simulator for a car park, which has only one entry, 
** one exit and 10 parking bays. The entry allows only one car to enter the car 
** park at a time and the exit only allows a car to leave at a time. When a car 
** arrives, it randomly picks up a parking bay to park.
**
** Author: Jackson Powell - n8600571
** -------------------------------------------------------------------------*/

#include "car_park.h"

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

int insertPointer = 0, removePointer = 0;

/* 
 * car_enter_park allows for the insertion of new cars into
 * the car park which is an array of type car, only if they can fit.
 * @param item is a specific car datatype to be inserted.
 */
int car_enter_park(car item)
{
    //Acquire Empty Semaphore
	sem_wait(&empty);
	
	//Acquire mutex lock to protect car_park
	pthread_mutex_lock(&mutex);
	
	//Try to select an empty parking space.
	do {
		insertPointer = rand() % CAR_PARK_SIZE;
	} while(car_park[insertPointer].carId[0] != '\0');
	
	//Insert the car and print the placement to screen.
	car_park[insertPointer] = item;
	printf("Car %s parked in bay %d.\n", 
		item.carId, (insertPointer+1));
	
	//Recognise when car_park is full, and display.
	int i;
	for(i = 0; i < CAR_PARK_SIZE; i++) {
		if(car_park[i].carId[0] == '\0') {
			break;
		} 
		if(i == CAR_PARK_SIZE - 1) {
			printf("The car park is full.\n");
		}
	}

	//Release mutex lock and full semaphore
	pthread_mutex_unlock(&mutex);
    sem_post(&full);

	return 0;
}

/* 
 * car_enter_park allows for the removal of cars from the
 * car park which is an array of type car, only if there is
 * a car to remove.
 */
int car_exit_park()
{
	//Acquire Full Semaphore
	sem_wait(&full);

	//Acquire mutex lock to protect car_park
	pthread_mutex_lock(&mutex);
	
	//Try to select a filled parking space.
	do {
		removePointer = rand() % CAR_PARK_SIZE;
	} while(car_park[removePointer].carId[0] == '\0');
	
	// Calculate time taken by a request
	struct timespec displayTime;
	clock_gettime(CLOCK_REALTIME, &displayTime);

	// Calculate time it took
	double elapsed = ((displayTime.tv_sec - car_park[removePointer].arriveTime.tv_sec)
	  + ((displayTime.tv_nsec - car_park[removePointer].arriveTime.tv_nsec) / CLOCK_PRECISION));
	
	printf("Car %s departed from bay %d and stayed for %.2f seconds.\n", 
		car_park[removePointer].carId, (removePointer+1), elapsed);
	
	car_park[removePointer] = EMPTY_PARK;
	
	//Recognise when car_park is empty, and display.
	int i;
	for(i = 0; i < CAR_PARK_SIZE; i++) {
		if(car_park[i].carId[0] != '\0') {
			break;
		} 
		if(i == CAR_PARK_SIZE - 1) {
			printf("The car park is empty.\n");
		}
	}

	//Release mutex lock and empty semaphore
	pthread_mutex_unlock(&mutex);
	sem_post(&empty);

	return 0;
}

/* 
 * The main thread that spawns all other threads and executes the 
 * main function of the program.
 */
int main(int argc, char *argv[])
{
	//Initialise Empty Parks
	int i = 0;
	for(i = 0; i < CAR_PARK_SIZE; i++) {
		car_park[i] = EMPTY_PARK;
	}
	
	//Introduction to the program.
	printf("\nWelcome to the car park simulator.\n");
	printf("Press p or P followed by return to display the state of the car park.\n");
	printf("Press q or Q followed by return to terminate the simulation.\n");
	printf("Press return to start the simulation.\n");
	char start_key;
	
	while(start_key != '\n') {
		start_key = getchar();
	}

	//Initialize the the locks
	pthread_mutex_init(&mutex, NULL);
	sem_init(&empty, 0, CAR_PARK_SIZE);
	sem_init(&full, 0, 0);
	srand(time(0));

	//Create the arrival, departure and monitor threads
	pthread_t arrivaltid; 
	pthread_attr_t arrivalattr;
	pthread_attr_init(&arrivalattr);
	pthread_create(&arrivaltid, &arrivalattr, arrival, NULL); //Producer Thread

	pthread_t departuretid;
	pthread_attr_t departureattr;
	pthread_attr_init(&departureattr);
	pthread_create(&departuretid, &departureattr, departure, NULL); //Consumer Thread

	pthread_t monitortid;
	pthread_attr_t monitorattr;
	pthread_attr_init(&monitorattr);
	pthread_create(&monitortid, &monitorattr, monitor, NULL); //Monitor Thread
	
	//Join the threads so only after input has completed,
	//does the program end.
	pthread_join(monitortid, NULL);
	
	return 0;
}

/* 
 * The arrival (producer) thread that handles the introduction of new
 * cars to the car park simulator. Operates on a 0.5 sleep loop.
 */
void *arrival(void *param)
{
	car newCar;

	while(TRUE)
	{
		// Calculate time taken by a request
		struct timespec arrivingTime;
		clock_gettime(CLOCK_REALTIME, &arrivingTime);
		
		//Generate a 6 character code for the car.
		//char finalString[6];
		
		int i;
		for(i = 0; i < 3; i++) {
			newCar.carId[i] = (char)((rand() % 26) + 65);
		}
		for(i = 3; i < 6; i++) {
			newCar.carId[i] = (char)((rand() % 10) + 48);
		}
		
		newCar.arriveTime = arrivingTime;

		//If random number below between 0-100 is also between 0-60. (60%)
		//then we allocate a parking place with new car. (if possible)
		if((rand() % 100) <= 60) {
			if(car_enter_park(newCar))
				fprintf(stderr, "Error");
		}

		struct timespec tim, tim2;
		tim.tv_sec  = 0;
		tim.tv_nsec = 500000000L; //Equal to half a second.
		
		//Sleep for half a second.
		nanosleep(&tim, &tim2);
	}

}

/* 
 * The departure (consumer) thread that handles the removal of existing
 * cars from the car park simulator. Operates on a 0.5 sleep loop.
 */
void *departure(void *param)
{
	while(TRUE)
	{
		//If random number below between 0-100 is also between 0-40. (40%)
		//then we remove a car from a parking place. (if possible)
		if((rand() % 100) <= 40) {
			if(car_exit_park())
				fprintf(stderr, "Error Consuming");
		}
		
		struct timespec tim, tim2;
		tim.tv_sec  = 0;
		tim.tv_nsec = 500000000L; //Equal to half a second.
		
		//Sleep for half a second.
		nanosleep(&tim, &tim2);
	}
}

/* 
 * The monitor thread handles all input from the user and then acts 
 * accordingly to exit the program 'q'/'Q' or show the current car park
 * state 'p'/'P'.
 */
void *monitor(void *param)
{
	int exit_program = 0;
	char input;

	//While user has not selected to quit, using single character
	//input, thread continues to wait.
	while(exit_program != TRUE)
	{
		scanf(" %c", &input);
		if(input == 'p' || input == 'P') {
			display_car_park();
		} else if(input == 'q' || input == 'Q') {
			exit_program = TRUE;
			printf("\nThe simulation has terminated.\n");
		}
	}
}

/* 
 * Function that allows quick displaying of the car park state by
 * iterating through the car spaces and displaying what is currently
 * being used and by whom.
 */
void display_car_park()
{
	int i;
	printf("Car park state:\n");
	
	// Calculate time current time.
	struct timespec displayTime;
	clock_gettime(CLOCK_REALTIME, &displayTime);
	double elapsed;

	//Prints each car slot appropriately.
	for(i = 0; i < CAR_PARK_SIZE; i++) {
		if(car_park[i].carId[0] == '\0') {
			printf("%d: Empty\n", (i+1));
		} else {
			//Calculate total time of each car, and print.
			elapsed = ((displayTime.tv_sec - car_park[i].arriveTime.tv_sec)
				+ ((displayTime.tv_nsec - car_park[i].arriveTime.tv_nsec) / CLOCK_PRECISION));
			printf("%d: %s (has parked for %.2f seconds)\n", (i+1), 
				car_park[i].carId, elapsed);
		}
	}
}