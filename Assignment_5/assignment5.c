#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <string.h>
#define PHIL_COUNT 5

/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */
int randomGaussian(int mean, int stddev);

/*
 * this function will be ran for each child process (each philosopher)
 * we will pass in the philosopher id, 
 * the possible choices the philosopher can make, and the sem_ID
 * returns and integer indicating errno
 */
int philosopher_begin_dinner(int phil_id, int sem_ID);

/*
 * main function that creates all the philosophers (forks 5 children)
 */
int main(int argc, char *argv[]){

	int errno_copy;

	/* 
	 * create sembufs to set the dinner table 
	 * (initialize semiphores to 1, like puting down the chopsticks) 
	 */
	struct sembuf set_table[5] = {
		{0, 1, 0},
		{1, 1, 0},
		{2, 1, 0},
		{3, 1, 0},
		{4, 1, 0}
	};

	/* create semiphores */
	int sem_ID = semget(IPC_PRIVATE, 5, IPC_CREAT | IPC_EXCL | 0600);

	if (sem_ID == -1) {
		errno_copy = errno;
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;

	} 

	/* set the dinner table */
	if (semop(sem_ID, set_table, 5) == -1) {
		errno_copy = errno;
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;
	}

	/* Loop and fork 5 new children */
	for (int i = 0; i < PHIL_COUNT; i++) {

		switch (fork()) {
			/* check if fork failed*/
			case -1:
				errno_copy = errno;
				fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
				return errno_copy;
			/* we are a philosopher, produce a random seed and begin dinner*/
			case 0:
				struct timespec time_seed;
				clock_gettime(CLOCK_MONOTONIC, &time_seed);
				srand(time_seed.tv_nsec);
				philosopher_begin_dinner(i, sem_ID);
				return 0;

			/* we are parent, so continue forking */
			default:
				break;

		}
	}

	/* wait on children */
	int status;
	while (wait(&status) != -1) {
		if (status != 0) {
			errno_copy = errno;
			fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
			return errno_copy;
		}
	}

	/* close semiphores */
	if (semctl(sem_ID, 0, IPC_RMID) == -1) {
		errno_copy = errno;
		fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
		return errno_copy;
	}

	return 0;
}

int philosopher_begin_dinner(int phil_id, int sem_ID) {

	int cycles = 0;
	int think_time = 0;
	int eat_time = 0;
	int errno_copy = 0;

	/* 
	 * create sembufs to pick up and put down chopsticks for each philosopher 
	 * picking up will decrement the chopstick count for the left and right chopstick 
	 * putting down will increment the chopstick count for the left and right chopstick 
	 * ensure philosophers share one left and right chopstick with another philosopher 
	 */
	struct sembuf pick_up_chops[2] = {{phil_id, -1, 0}, {(phil_id + 1) % 5, -1, 0}};
	struct sembuf put_down_chops[2] = {{phil_id, 1, 0}, {(phil_id + 1) % 5, 1, 0}};

	/* loop until philosphers are done eating */
	while(eat_time < 100) {

		/* set time to think, then sleep for that time */
		int time_to_think = randomGaussian(11, 7);
		if(time_to_think < 0) time_to_think = 0;
		printf("Philospher %d thinking for %d seconds (total = %d)\n", phil_id, time_to_think, think_time);
		sleep(time_to_think);
		think_time += time_to_think;

		/* set time to eat, perform semop to pick up both chopsticks */
		int time_to_eat = randomGaussian(9, 3);
		if(time_to_eat < 0) time_to_eat = 0;


		if (semop(sem_ID, pick_up_chops, 2) == -1) {
			errno_copy = errno;
			fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
			return errno_copy;
		}

		/* sleep to simulate eat time */
		printf("Philospher %d eating for %d seconds (total = %d)\n", phil_id, time_to_eat, eat_time);
		sleep(time_to_eat);
		eat_time += time_to_eat;

		/* done eating, put chopsticks back down */

		if (semop(sem_ID, put_down_chops, 2) == -1) {
			errno_copy = errno;
			fprintf(stderr, "ERRNO %d: %s \n", errno_copy, strerror(errno_copy));
			return errno_copy;
		}


		cycles++;
	}

	printf("Philospher %d done with meal. Thought for %d seconds, ate for %d seconds over %d cylces \n", phil_id, think_time, eat_time, cycles);
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
