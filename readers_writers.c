#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
#include "check.h"
#include <syscall.h>

#define BUFFER_SIZE 3
#define PROBABILITY_WRITE 30 // 30% OF CHANCE TO WRITE
/* buffer counter*/
int counter;

/* The mutex lock for the writer*/
pthread_mutex_t rw_mutex;
/* mutual exclusion to update a variable (read_count --> current number
of readers*/
pthread_mutex_t mutex;


/* the semaphores */
sem_t full, empty;
/* current number of readers */
int read_count=0;
/*shared data*/
int shared_data;

pthread_t tid;       //Thread ID
pthread_attr_t attr; //Set of thread attributes

void *writer(void *param);
void *reader(void *param);

void initializeData() {
   /* Create the mutex lock */
   pthread_mutex_init(&mutex, NULL);
   /* Create the full semaphore and initialize to 0 */
   sem_init(&full, 0, 0);
   /* Create the empty semaphore and initialize to BUFFER_SIZE */
   sem_init(&empty, 0, BUFFER_SIZE);
   /* Get the default attributes */
   pthread_attr_init(&attr);
   /* init buffer */
   shared_data = 1;
}

void *reader(void *param) {
   int count;
   for(count=0; count<4 ; count++) {
      sem_wait(&empty);             // Only 3 threads can read critical section at the same time
      // pthread_mutex_lock(&mutex);
      pthread_mutex_lock(&rw_mutex); // A reader need to wait a writer stop updating value
		  printf("\n(Thread: %02d - Iteration: %02d) --  read: %02d",param,count,shared_data);
      pthread_mutex_unlock(&rw_mutex);
      sleep(rand()%3+1);
      // pthread_mutex_lock(&mutex);
      sem_post(&empty);
      printf("\n(Thread: %02d - Stop reading: exited critical section)",param);
   }
}

void *writer(void *param) {
   // printf("\n(%07d) -- write %05d",syscall(SYS_gettid),shared_data);

   sleep(1);
   while(1) {
     int probability_of_write = rand()%100;
     if(probability_of_write < PROBABILITY_WRITE) {
       pthread_mutex_lock(&rw_mutex); // Only one writer can join critical section
       shared_data++;
       printf("\n(Thread:  %c -         WRITE) -- write: %02d",(65 + param),shared_data);
       pthread_mutex_unlock(&rw_mutex);
       printf("\n(Thread:  %c - Stop writing: exited critical section)",(65 + param),shared_data);
     } else {
       // Writer don't do nothing
     }


		sleep(1);
   }


}

int main(int argc, char *argv[]) {
	srand(time(0));

   initializeData();

   int mainSleepTime = atoi(argv[1]); /* Time in seconds for main to sleep */
   int numWriter = atoi(argv[2]); /* Number of writers*/
   int numReader = atoi(argv[3]); /* Number of readers*/

   /* Initialize the app */
   pthread_mutex_init(&rw_mutex, NULL);
   pthread_mutex_init(&mutex, NULL);
   read_count = 0;

   pthread_t * thread  =  malloc ((numWriter+numReader)*sizeof(pthread_t));
   /* Create the writer threads */
	int i;

   /* Create the reader threads */
   for(i = 0; i < numReader; i++) {
      /* Create the thread */
      thread[i]=pthread_create(&tid,&attr,reader,i);
   }

   for(i = 0; i < numWriter; i++) {
      thread[numWriter+i]=pthread_create(&tid,&attr,writer,i);
   }


   /* Sleep for the specified amount of time in milliseconds */
   sleep(mainSleepTime);

   /* Exit the program */
   printf("\nExit the program\n");
   exit(0);
}
