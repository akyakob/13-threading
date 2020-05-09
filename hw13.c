//  This fills ram with +3 sequential integers
//  student file
//
//   gcc -g -O0 -std=c99 hw13.c -lpthread -o hw13 -Wall -pedantic
//  valgrind --tool=memcheck --leak-check=yes ./hw13 -f -s

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#define EN_TIME
#include "Timers.h"
#include "ClassErrors.h"

/*--------------------------------------------------------------------------
  Local data structures and defines 
--------------------------------------------------------------------------*/
// Use the larger data size of the Linux cluster, smaller for
// a typical PC. 
// guarantees divisible by 2/3/4/5/7/8  Note: this number is VERY large
// and MAY cause overflow issues
#define DATA_SIZE           (136*3*5*7*146*512)
#define VALGRIND_DATA_SIZE  (30*3*5*7*8*1024)


// Maximum number of threads 
#define MAX_THREADS     (8)

// The number of iterations to slow down thread execution
#define DELAY_LOOPS_EXP        (5)     

// The percentage rate to update thread progress
#define STATUS_UPDATE_RATE (10)

// Thread information control structure 
  struct ThreadData_s {
     int threadID;      // Contains the thread ID number 0..n
     int segSize;       // The amount of total work for this thread
                        // threadID*segSize is the starting index of
                        // the data to initialize
     int *dataPtr;      // Pointer to the one contagious array buffer
                        // that each thread will work on
     int trackStatus;   // Flag to identify if status updates should be reported
     int verbose;       // Flag to indicate if the task should run in verbose mode
  };

  
/* Function prototypes */
void *do_process(void *data);

/* Used to control access to the progress counter */
   volatile int processed = 0;
   pthread_mutex_t lock;
   int rc_codes[MAX_THREADS]; //return codes array of size of num threads


int main(int argc, char *argv[]) {
   /*------------------------------------------------------------------------
     General purpose variables 
   ------------------------------------------------------------------------*/
  // DECLARE_TIMER(timer); 
 
   time_t  wallTime = time(NULL);;    // Used to report wall execution time.
   
   int* int_array;

   if (pthread_mutex_init(&lock, NULL)) {
	printf("mutex initialization failed in main\n");
	exit(99);
   }
  
 //  pthread_mutex_t mut_thread;
   /*------------------------------------------------------------------------
     Thread process information
   ------------------------------------------------------------------------*/
   void *rcp; //process return code
   pthread_t th_array[MAX_THREADS];
   struct ThreadData_s threadData[MAX_THREADS];
   
   /*------------------------------------------------------------------------
      UI variables with sentential values
   ------------------------------------------------------------------------*/
   int rc;
//   int opterr;
   int verbose = 0; 
   int status;
   int dataSize = DATA_SIZE;
   int numThreads = 0;
  
   int option_index = 0;
   char *getoptOptions = "t:sfv";   

   /*------------------------------------------------------------------------
     These variables are used to control the getopt_long_only command line 
     parsing utility.  
   ------------------------------------------------------------------------*/
   struct option long_options[] = {
	{"threads", required_argument, 0, 'o'}, //num threads, required
	{"status", no_argument, 0, 's'},	//display thread progress, optional
	{"fast", no_argument, 0, 'f'},		//shorter data run for Valgrind, optional
	{"verbose", no_argument, 0, 'v'},
	{"verb", no_argument, 0, 'v'},
	{0, 0, 0, 0}
   };
 
   opterr = 1;
   while ((rc = getopt_long_only(argc, argv, getoptOptions, long_options,
						&option_index)) != -1) {
	switch(rc)
	{ 
	  case 't':
	  numThreads = atoi(optarg);
	  if (numThreads > 8 || numThreads < 1) {
		printf("Number of threads should be greater than 0 and less than 9\n");
		exit(-99); }
	  break;

	  case 's':
	  status = 1;
	  break;

	  case 'f':
	  dataSize = VALGRIND_DATA_SIZE;
	  break;

	  case 'v':
	  verbose = 1;
	  break;

	  case '?':
	  break;
 
	  default:
	   printf("Internal error: undefined option %0xX\n", rc);
	   exit(PGM_INTERNAL_ERROR);
        } //end switch
   } //end while rc

   /*------------------------------------------------------------------------
     Check for command line syntax errors
   ------------------------------------------------------------------------*/
   if ((optind < argc) || numThreads == 0 ){
      fprintf(stderr, "This program demonstrates threading performance.\n");
      fprintf(stderr, "usage: hw13 -t[hreads] num [-s[tatus]] [-f[ast]] [-v[erbose]] \n");
      fprintf(stderr, "Where: -t[hreads] num - number of threads 1 to %d,required\n", MAX_THREADS);
      fprintf(stderr, "       -s[tatus]      - display thread progress, optional\n"); 
      fprintf(stderr, "       -v[erbose]     - verbose flag, optional\n");
      fprintf(stderr, "       -f[ast]        - shorter run for Valgrind, optional\n");
      fprintf(stderr, "eg: hw13 -t 3 -status\n");
      fflush(stderr);
      return(PGM_SYNTAX_ERROR);
   } /* End if error */

   /* Get space for the data */
   int_array = (int*)malloc(dataSize*sizeof(int));
   if(int_array == NULL) {
	printf("int array malloc failed\n");
	exit(-99); 
	}
   
   
   // Print message before starting the timer
   printf("\nStarting %d threads generating %d numbers\n\n", numThreads, dataSize);   

 
   
   // Spin up N threads
   for(int i = 0; i < numThreads; i++) {
      // Build the thread specific information
      threadData[i].threadID = i;
      threadData[i].segSize = dataSize/numThreads;
      threadData[i].dataPtr = &int_array[i*(dataSize/numThreads)];
      threadData[i].trackStatus = status;
      threadData[i].verbose = verbose;
      
      // Start the thread
      int tc = pthread_create(&th_array[i], NULL, do_process, &threadData[i]);
      if (tc) {
	fprintf(stderr, "Failed to start thread tc: %d\n", tc);
	exit(99);
      }

      if (verbose) {
         fprintf(stdout, "Thread:%d  ID:%ld started\n", i, (unsigned long int)th_array[i]);
      }
   } // End threads  
 
   /* Print out the progress status */
   if (status == 1) {
	pthread_mutex_lock(&lock);
	while(processed < dataSize) 
	{
	   printf("Processed: %d lines %3.0f%% complete\n", processed, ((float)processed/(float)dataSize)*100);
	   pthread_mutex_unlock(&lock);
	   sleep(1);
	   pthread_mutex_lock(&lock);
	} // end while
	pthread_mutex_unlock(&lock);
   } // end if status

   /* Wait for all processes to end */
   for(int i = 0; i< numThreads; i++) {
 	pthread_join(th_array[i], &rcp);
   } // End threads  
   
   

   time_t  wallTimeEnd = time(NULL);;    // Used to report wall execution time.
   printf("Total wall time = %d sec\n", (int)(wallTimeEnd-wallTime));

   

   printf("Verifying results...  ");
   for (int i = 0; i < dataSize; i++) {
       if (int_array[i] != 3*i) {
         printf("Error int_array[%d]= %d != %d\n", i, int_array[i], 3*i); 
          exit(PGM_INTERNAL_ERROR);}
   } // End verification
   printf("success\n\n");

   
   // Clean up
free(int_array);
pthread_exit(NULL);
return(0); 
   } // End main

   
/****************************************************************************
  This threading process will initialize parts of a very large array by 3's
  It contains code to SLOW execution down so that status updates can be easily
  seen.  The function prototype is defined by pthread so we MUST use it.
  
  void *do_process(void *data)
  Where: void *data - pointer to some user defined data structure
                      We will use struct ThreadData_s
  Returns: void *   - pointer to some user defined return code structure
                      We will use an integer array
  Errors: none
 
****************************************************************************/
void *do_process(void *data) {
   struct ThreadData_s* data_0 = data;
   int counter = 0;
//   volatile int processed = 0;
//   pthread_mutex_t lock;
   int lim = (data_0->segSize) * (STATUS_UPDATE_RATE/100);
   int start = (data_0->segSize) * (data_0->threadID);
 
   if (pthread_mutex_init(&lock, NULL)) {
	printf("mutex initialization failed in do_process\n");
	exit(99);
   }

   // Print out the thread status
   if (data_0->verbose) {
      fprintf(stdout, "Thread:%d  track status:%d  seg size:%dKB  data ptr:%p\n", data_0->threadID, data_0->trackStatus, data_0->segSize, (void *)data_0->dataPtr );
      fflush(stdout);
   } // End verbose
   
   // Process all the data in the portion of the array

      // Print out the thread status
      if (data_0->verbose) {
         fprintf(stdout, "Thread: %d\n", data_0->threadID);
         fflush(stdout);
         } // End verbose
   for (int i = start; i < start+(data_0->segSize); i++) {
      data_0->dataPtr [i-start] = 3*i;
     
      // Slow the CPU
      int delay = 1<<DELAY_LOOPS_EXP;
      while (delay)
         {
         delay--;
         }
     
      counter++;
      // Track status if required
	if((data_0->trackStatus) && counter>=lim) {
	   pthread_mutex_lock(&lock);
	   processed += counter;
	   counter = 0;
	   pthread_mutex_unlock(&lock);
	      
 /* 
      // Print out the thread status
      if (data_0->verbose) {
         fprintf(stdout, "%d", data_0->threadID);
         fflush(stdout);
         } // End verbose  */
      } // End if
   } // End i
  
   // There might be some status left to update
   if (data_0->trackStatus) {
	pthread_mutex_lock(&lock);
	processed += counter;
	pthread_mutex_unlock(&lock);
   }

   // Return the task ID number + 10
   rc_codes[data_0->threadID] = data_0->threadID + STATUS_UPDATE_RATE;
   pthread_exit(&rc_codes[data_0->threadID]);
   //return(       );
} // End do_process
