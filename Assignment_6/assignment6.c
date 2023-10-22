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
	int phil_count;
	int *chops;
	pthread_mutex_t *mtx;
	pthread_cond_t *cv;
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
 * this function call simulates a decrement semop operation (wait equivelent)
 * it takes in the philosopher information, and will 
 * attempt to pick up the chopsticks if available
 * 
 * returns error status if an error occurs
 */
int pick_up_chops(struct phil_info *);

/*
 * this function call simulates an increment semop operation (signal equivelent)
 * it takes in the philosopher information, and will 
 * attempt to put down the chopsticks if available
 * 
 * returns error status if an error occurs
 */
int put_down_chops(struct phil_info *);

/*
 * main function that creates all the philosophers (creates 5 new threads)
 */
int main(int argc, char *argv[]){

	int errno_copy;
	int phil_count = PHIL_COUNT;

	pthread_mutex_t *lock = calloc(1, sizeof(pthread_mutex_t));
	if ((errno_copy = pthread_mutex_init(lock, NULL)) != 0) {
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;
	}

	pthread_cond_t *cv = calloc(1, sizeof(pthread_cond_t));

	if ((errno_copy = pthread_cond_init(cv, NULL)) != 0) {
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;
	}

	int *chopsticks = calloc(phil_count, sizeof(int));

	/* 
	 * set chopstick values to 1 initially 
	 * one means the chopsticks are present
	 * initially, the table has all the chopsticks present
	 */
	for(int i = 0; i < phil_count; i++) {
		chopsticks[i] = 1;
	}

	pthread_t phil_threads[phil_count];

	for (int i = 0; i < phil_count; i++) {

		/* create the philosopher information to pass to each thread*/
		struct phil_info *info = malloc(sizeof(struct phil_info));

		info->phil_id = i;
		info->phil_count = phil_count;
		info->chops = chopsticks;
		info->mtx = lock;
		info->cv = cv;


		/* maybe put in thread function, not sure */
		/* set random seed */
		struct timespec time_seed;
		clock_gettime(CLOCK_MONOTONIC, &time_seed);
		srand(time_seed.tv_nsec);

		int errno_copy = pthread_create(
			&phil_threads[i], 
			NULL, 
			(void *) philosopher_begin_dinner, 
			(void *) info
		);

		if (errno_copy != 0) {
			fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
			return errno_copy;
		}
	}


	/* wait until all the threads are done */
	for (int i = 0; i < phil_count; i++) {
		if((errno_copy = pthread_join(phil_threads[i], NULL)) != 0) {
			fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
			return errno_copy;
		}
	}

	pthread_mutex_destroy(lock);
	pthread_cond_destroy(cv);
	free(chopsticks);
	free(lock);
	free(cv);

	return 0;
}

int philosopher_begin_dinner(void *philosopher_info) {

	int cycles = 0;
	int think_time = 0;
	int eat_time = 0;
	int errno_copy = 0;
	struct phil_info *info = (struct phil_info *)philosopher_info;


	/* loop until philosphers are done eating */
	while (eat_time < EAT_TIME) {

		/* set time to think, then sleep for that time */
		int time_to_think = randomGaussian(11, 7);
		if (time_to_think < 0) time_to_think = 0;
		printf("Philospher %d thinking for %d seconds (total = %d)\n", info->phil_id, time_to_think, think_time);
		sleep(time_to_think);
		think_time += time_to_think;

		/* attempt to pick up chopsticks*/
		if ((errno_copy = pick_up_chops(info)) != 0) {
			fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
			return errno_copy;
		}

		/* set time to eat, perform semop to pick up both chopsticks */
		int time_to_eat = randomGaussian(9, 3);
		if(time_to_eat < 0) time_to_eat = 0;
		/* sleep to simulate eat time */
		printf("Philospher %d eating for %d seconds (total = %d)\n", info->phil_id, time_to_eat, eat_time);
		sleep(time_to_eat);
		eat_time += time_to_eat;

		/* put down chopsticks */
		if ((errno_copy = put_down_chops(info)) != 0) {
			fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
			return errno_copy;
		}

		cycles++;
	}

	printf("Philospher %d done with meal. Thought for %d seconds, ate for %d seconds over %d cylces \n", info->phil_id, think_time, eat_time, cycles);
	free(philosopher_info);
	return 0;

}


int pick_up_chops(struct phil_info *info) {

	/* these dont need to be protected, they never change */
	int phil_id = info->phil_id;
	int phil_count = info->phil_count;
	int errno_copy = 0;

	/* lock the mutex */
	if ((errno_copy = pthread_mutex_lock(info->mtx)) != 0) {
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;
	}

	/* if either chopstick isn't available, wait until they are */
	while (info->chops[phil_id] == 0 ||
		info->chops[(phil_id + 1) % phil_count] == 0) {
		if ((errno_copy = pthread_cond_wait(info->cv, info->mtx)) != 0) {
			fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
			return errno_copy;
		}
	}

	/* simulate taking the chopsticks by decrementing the value */
	info->chops[phil_id]--;
	info->chops[(phil_id + 1) % phil_count]--;

	/* unlock mutex */
	if ((errno_copy = pthread_mutex_unlock(info->mtx)) != 0) {
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;
	}

	return 0;
}

int put_down_chops(struct phil_info *info){

	/* these dont need to be protected, they never change */
	int phil_id = info->phil_id;
	int phil_count = info->phil_count;
	int errno_copy = 0;

	/* lock the mutex */
	if ((errno_copy = pthread_mutex_lock(info->mtx)) != 0) {
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;
	}

	/* simulate putting chopsticks back by incrementing */
	info->chops[phil_id]++;
	info->chops[(phil_id + 1) % phil_count]++;

	/* broadcast this info to all philosophers so they can all try to eat */
	if ((errno_copy = pthread_cond_broadcast(info->cv)) != 0) {
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;
	}

	/* unlock mutex */
	if ((errno_copy = pthread_mutex_unlock(info->mtx)) != 0) {
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;
	}

	return 0;
}

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
