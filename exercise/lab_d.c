/*---------------------------------------------------------------------------
  Create a single thread and use pthread_exit()  
  student copy
  
  gcc -g -std=c99 lab14a.c -lpthread -o lab14a
---------------------------------------------------------------------------*/
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> //For sleep().  
#include <pthread.h> //For threads
#include <time.h>

void *mySimpleThread(void *name);

// Global variables
int taskRC[3] = {-1,-1,-1};

volatile int processed = 0;
pthread_mutex_t lock;

int main(int argc, char *argv[]) { 

time_t start = time(NULL);

   // Thread data structure
   pthread_t thread_id; 
   pthread_t thread_id2; 
   pthread_t thread_id3; 
   
   // Pointer to the process return code 
   void *rcp;
   
   // Process index id number 
   int num[3] = {1,2,3};
   
   int rc;

  if(pthread_mutex_init(&lock, NULL)) {
	printf("mutex initialization failed\n");
	return 1;
	}

   printf("Main: Starting thread...\n"); 
   // Student should add the thread create call here
   rc = pthread_create(&thread_id, NULL, mySimpleThread, &num[0]); 
   if (rc) {
      printf("Thread failed to start rc= %d\n", rc);
      exit(99);
   } // End if rc
   rc = pthread_create(&thread_id2, NULL, mySimpleThread, &num[1]); 
   if (rc) {
      printf("Thread failed to start rc= %d\n", rc);
      exit(99);
   } // End if rc
   rc = pthread_create(&thread_id3, NULL, mySimpleThread, &num[2]); 
   if (rc) {
      printf("Thread failed to start rc= %d\n", rc);
      exit(99);
   } // End if rc
   printf("Main: All threads started...\n"); 

   // Student should add wait for the thread to finish
   // when instructed. Always check the join RC as you
   // may get memory leaks otherwise.
   pthread_mutex_lock(&lock);
   while(processed<6) {
       printf("Progress of processed variable: %d\n", processed);
	pthread_mutex_unlock(&lock);
	sleep(1);
	pthread_mutex_lock(&lock);
   }

   pthread_mutex_unlock(&lock);


   pthread_join(thread_id, &rcp); 
   printf("rcp: %d\n", *(int *)rcp);  
   pthread_join(thread_id2, &rcp); 
   printf("rcp: %d\n", *(int *)rcp);  
   pthread_join(thread_id3, &rcp); 
   printf("rcp: %d\n", *(int *)rcp);  
   
   // Print status in the main routine 
   for (int i = 0; i < 2; i++){
      printf("Main will sleep 1 second. \n"); 
      sleep(1); 
   }

time_t end  = time(NULL);
printf("Total wall time used in main thread %f\n", difftime(end, start));   
   printf("Exiting main thread\n");
   pthread_exit(NULL);
}

/*---------------------------------------------------------------------------
  A thread that prints out it's ID number and sleeps in a loop,
  printing status
---------------------------------------------------------------------------*/
void *mySimpleThread(void *num) { 
   for (int i = 0; i < 6; i++){
      printf("     Thread %d, will sleep 1 second. \n", *(int *)num); 
	pthread_mutex_lock(&lock);
	processed++;
	pthread_mutex_unlock(&lock);
      sleep(1); 
   }
   int num_l = *(int *)num;
   taskRC[num_l-1] = *(int *)num;
   return(&taskRC[num_l-1]); 
} 
