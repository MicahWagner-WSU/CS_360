#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#define PHIL_COUNT 5

/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */

int randomGaussian(int mean, int stddev);

void philosopher_begin_dinner(int phil_id, struct sembuf **phil_choices, int sem_ID);


int main(int argc, char *argv[]){

	struct sembuf pick_up_chops1[2] = {{0, -1, 0}, {1, -1, 0}};
	struct sembuf put_down_chops1[2] = {{0, 1, 0}, {1, 1, 0}};

	struct sembuf pick_up_chops2[2] = {{1, -1, 0}, {2, -1, 0}};
	struct sembuf put_down_chops2[2] = {{1, 1, 0}, {2, 1, 0}};

	struct sembuf pick_up_chops3[2] = {{2, -1, 0}, {3, -1, 0}};
	struct sembuf put_down_chops3[2] = {{2, 1, 0}, {3, 1, 0}};

	struct sembuf pick_up_chops4[2] = {{3, -1, 0}, {4, -1, 0}};
	struct sembuf put_down_chops4[2] = {{3, 1, 0}, {4, 1, 0}};

	struct sembuf pick_up_chops5[2] = {{4, -1, 0}, {0, -1, 0}};
	struct sembuf put_down_chops5[2] = {{4, 1, 0}, {0, 1, 0}};

	struct sembuf set_table[5] = {
		{0, 1, 0},
		{1, 1, 0},
		{2, 1, 0},
		{3, 1, 0},
		{4, 1, 0}
	};

	int sem_ID = semget(IPC_PRIVATE, 5, IPC_CREAT | IPC_EXCL | 0600);

	semop(sem_ID, set_table, 5);

	struct sembuf *philosopher_choices[10] = {
		pick_up_chops1,
		pick_up_chops2, 
		pick_up_chops3, 
		pick_up_chops4, 
		pick_up_chops5, 
		put_down_chops1,
		put_down_chops2,
		put_down_chops3,
		put_down_chops4,
		put_down_chops5
	};

	int errno_copy;

	for (int i = 0; i < PHIL_COUNT; i++) {
		switch (fork()) {
			case -1:
				errno_copy = errno;
				perror("ERROR");
				return errno_copy;
			case 0:
				struct timespec time_seed;
				clock_gettime(CLOCK_MONOTONIC, &time_seed);
				srand(time_seed.tv_nsec);
				philosopher_begin_dinner(i, philosopher_choices, sem_ID);
				return 0;

			default:
				break;

		}
	}

	int status;
	while (wait(&status) != -1);

	/* code */
	return 0;
}

void philosopher_begin_dinner(int phil_id, struct sembuf **phil_choices, int sem_ID) {

	int cycles = 0;
	int think_time = 0;
	int eat_time = 0;

	while(eat_time < 100) {

		int time_to_think = randomGaussian(11, 7);
		if(time_to_think < 0) time_to_think = 0;
		printf("Philospher %d thinking for %d seconds (total = %d)\n", phil_id, time_to_think, think_time);
		sleep(time_to_think);
		think_time += time_to_think;

		int time_to_eat = randomGaussian(9, 3);
		if(time_to_eat < 0) time_to_eat = 0;
		semop(sem_ID, phil_choices[phil_id], 2);

		printf("Philospher %d eating for %d seconds (total = %d)\n", phil_id, time_to_eat, eat_time);
		sleep(time_to_eat);
		eat_time += time_to_eat;

		semop(sem_ID, phil_choices[phil_id + 5], 2);

		cycles++;
	}

	printf("Philospher %d done with meal. Thought for %d seconds, ate for %d seconds over %d cylces \n", phil_id, think_time, eat_time, cycles);

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
