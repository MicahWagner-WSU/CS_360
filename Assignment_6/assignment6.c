#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#define PHIL_COUNT 5
#define EAT_TIME 100



struct phil_info {
	int phil_id;
	pthread_mutex_t *table;
};

/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */
int randomGaussian(int mean, int stddev);

/*
 * this function will be ran for each philosopher on a new thread
 * we will pass in the philosopher info
 * returns and integer indicating errno
 */
int philosopher_begin_dinner(void *phil_info);

/*
 * this function will allocate space for each chopstick on the heap
 * given the philosopher count, we will make a table with 
 * as many chopsticks as philosophers
 *
 * return pointer to the chopstick array
 */
// int *init_table(int phil_amount);

/*
 * main function that creates all the philosophers (creates 5 new threads)
 */
int main(int argc, char *argv[]){

	int errno_copy;

	pthread_mutex_t *table = calloc(PHIL_COUNT, sizeof(pthread_mutex_t));

	for (int i = 0; i < PHIL_COUNT; i++) {
		pthread_mutex_init(&table[i], NULL);
	}

	pthread_t *phil_threads = calloc(PHIL_COUNT, sizeof(pthread_t));

	for (int i = 0; i < PHIL_COUNT; i++) {

		struct phil_info *info = malloc(sizeof(struct phil_info));

		info->phil_id = i;
		info->table = table;

		/* maybe put in thread function, not sure */
		struct timespec time_seed;
		clock_gettime(CLOCK_MONOTONIC, &time_seed);
		srand(time_seed.tv_nsec);

		phil_threads[i] = pthread_create(
			&phil_threads[i], 
			NULL, 
			(void *) philosopher_begin_dinner, 
			(void *) info
		);
	}

	for (int i = 0; i < PHIL_COUNT; i++) {
		pthread_join(phil_threads[i], NULL);
	}

	return 0;
}

int philosopher_begin_dinner(void *info) {

	int cycles = 0;
	int think_time = 0;
	int eat_time = 0;
	int errno_copy = 0;


	/* loop until philosphers are done eating */
	while (eat_time < EAT_TIME) {

		/* set time to think, then sleep for that time */
		int time_to_think = randomGaussian(11, 7);
		if (time_to_think < 0) time_to_think = 0;
		//printf("Philospher %d thinking for %d seconds (total = %d)\n", phil_id, time_to_think, think_time);
		sleep(time_to_think);
		think_time += time_to_think;

		/* set time to eat, perform semop to pick up both chopsticks */
		int time_to_eat = randomGaussian(9, 3);
		if(time_to_eat < 0) time_to_eat = 0;


		/* sleep to simulate eat time */
		//printf("Philospher %d eating for %d seconds (total = %d)\n", phil_id, time_to_eat, eat_time);
		sleep(time_to_eat);
		eat_time += time_to_eat;


		cycles++;
	}

	//printf("Philospher %d done with meal. Thought for %d seconds, ate for %d seconds over %d cylces \n", phil_id, think_time, eat_time, cycles);
	return 0;

}

// int *init_table(int phil_amount) {
	
// }



/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */

int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5)) 
		return (int) floor(mu + sigma * cos(f2) * f1);
	else            
		return (int) floor(mu + sigma * sin(f2) * f1);
}
