#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_SLEEP_TIME 4 //maximum time (in sec)to sleep
#define NUM_OF_STUDENTS 3 //number of students
#define NUM_OF_INSTRUCTORS 2 //number of instructors
#define NUM_OF_SESSIONS 2 //number of flight sessions each student must take before exit

sem_t mutex ; //binary semaphore
int waiting_instructors, waiting_students ; //number of waiting instructors, students
sem_t instructors_q, students_q ; //queue for instructors, students
sem_t session_over ; //session over

void *instructor(void*) ;
void *student(void*) ;

int main(void)
{
	puts("CS149 Spring 2020 FlightSchool from Le Duy Vu") ;

	//Threads for instructors and students
	pthread_t instructors[NUM_OF_INSTRUCTORS] ;
	pthread_t students[NUM_OF_STUDENTS] ;

	//Initialize global variables
	waiting_instructors = waiting_students = 0 ;
	if (sem_init(&mutex, 0, 1) < 0 || sem_init(&instructors_q, 0, 0) < 0 ||
		sem_init(&students_q, 0, 0) < 0 || sem_init(&session_over, 0, 0) < 0)
	{
		puts("\nInitialize semaphore failed\n") ;
		exit(EXIT_FAILURE);
	}

	//Create instructor threads
	for (int i = 0; i < NUM_OF_INSTRUCTORS; i++)
	{
		if (pthread_create(&instructors[i], NULL, instructor, (void *)i) != 0)
		{
			printf("\nCreate thread instructors[%d] failed\n", i) ;
			fflush(NULL) ;
			exit(EXIT_FAILURE);
		}
	}

	//Create student threads
	for (int i = 0; i < NUM_OF_STUDENTS; i++)
	{
		if (pthread_create(&students[i], NULL, student, (void *)i) != 0)
		{
			printf("\nCreate thread students[%d] failed\n", i) ;
			fflush(NULL) ;
			exit(EXIT_FAILURE);
		}
	}

	//Join student threads
	for (int i = 0; i < NUM_OF_STUDENTS; i++)
	{
		if (pthread_join(students[i], NULL) != 0)
		{
			printf("\nJoin thread students[%d] failed\n", i) ;
			fflush(NULL) ;
			exit(EXIT_FAILURE);
		}
	}

	//Cancel instructor threads after student threads have finished
	for (int i = 0; i < NUM_OF_INSTRUCTORS; i++)
	{
		if (pthread_cancel(instructors[i]) != 0)
		{
			printf("\nCancel thread instructors[%d] failed\n", i) ;
			fflush(NULL) ;
			exit(EXIT_FAILURE);
		}
	}

	//Destroy semaphores
	if (sem_destroy(&mutex) < 0 || sem_destroy(&instructors_q) < 0 ||
		sem_destroy(&students_q) < 0 || sem_destroy(&session_over) < 0)
	{
		puts("\nDestroy semaphore failed\n") ;
		exit(EXIT_FAILURE);
	}

	puts("main: done") ;
	return EXIT_SUCCESS;
}

/*
 * Life cycle of an instructor.
 * @param param instructor ID
 */
void *instructor(void *param)
{
	int i = (int)param ;
	int taught = 0 ; //number of sessions taught by this instructor

	do
	{
		//Rest for a random amount of time before entering lounge
		unsigned int seed = time(NULL) ;
		int s = (rand_r(&seed) % MAX_SLEEP_TIME) + 1 ;
		printf("instructor[%d, %d]: rest for %d second(s)\n", i, taught, s) ;
		fflush(NULL) ;
		sleep(s) ;

		//Acquire lock and enter lounge
		sem_wait(&mutex) ;
		printf("instructor[%d, %d]: waiting_instructors (excluding me) = %d, waiting_"
				"students = %d\n", i, taught, waiting_instructors, waiting_students) ;
		fflush(NULL) ;
		waiting_instructors++ ;

		//If no student waiting, release lock and wait in q
		//Otherwise, notify a student
		if (waiting_students > 0)
			sem_post(&students_q) ;
		else
		{
			sem_post(&mutex) ;
			sem_wait(&instructors_q) ;
		}

		//Simulate teach_session() and release lock
		taught++ ;
		waiting_instructors-- ;
		seed = time(NULL) ;
		s = (rand_r(&seed) % MAX_SLEEP_TIME) + 1 ;
		printf("instructor[%d, %d]: teach a session for %d second(s)\n", i, taught, s) ;
		fflush(NULL) ;
		sleep(s) ;
		sem_post(&session_over) ;
		sem_post(&mutex) ;
	} while (1) ;

	pthread_exit(0) ;
}

/*
 * Life cycle of a student.
 * @param param student ID
 */
void *student(void *param)
{
	int i = (int)param ;

	//Loop for student to take enough sessions
	for (int j = 0; j < NUM_OF_SESSIONS; j++)
	{
		//Rest for a random amount of time before entering lounge
		unsigned int seed = time(NULL) ;
		int s = (rand_r(&seed) % MAX_SLEEP_TIME) + 1 ;
		printf("student[%d, %d]: rest for %d second(s)\n", i, j, s) ;
		fflush(NULL) ;
		sleep(s) ;

		//Acquire lock and enter lounge
		sem_wait(&mutex) ;
		printf("student[%d, %d]: waiting_instructors = %d, waiting_students (excluding "
				"me) = %d\n", i, j, waiting_instructors, waiting_students) ;
		fflush(NULL) ;
		waiting_students++ ;

		//If no instructor waiting, release lock and wait in q
		//Otherwise, notify an instructor
		if (waiting_instructors > 0)
			sem_post(&instructors_q) ;
		else
		{
			sem_post(&mutex) ;
			sem_wait(&students_q) ;
		}

		//Simulate take_session()
		printf("student[%d, %d]: learn to fly\n", i, j + 1) ;
		fflush(NULL) ;
		waiting_students-- ;
		sem_wait(&session_over) ;
	}

	pthread_exit(0) ;
}
