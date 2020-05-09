/******************************************************************************
* Timing Instrumentation Macros, provides
*   DECLARE_TIMER, START_TIMER, STOP_TIMER,PRINT_TIMER    (Original Timers.h)
*   BEGIN_REPEAT_TIMING, END_REPEAT_TIMING, PRINT_RTIMER 
*
* Usage:
*  To "enable" compile with -DTIMING
*  A warning will be printed when the timers are enabled.
*  All timer output is printed to stderr.
*
*  Based on Greg Semeraro's Timers.h
*  Extended by Juan C. Cockburn with looping code
*
*  Revised: 2/25/2015 (jcck)
*  02/21/2016   R. Repka    Added better bracketing
*  02/28/2016   R. Repka    Added a loop define macro
*  08/12/2016   R. Repka    Very minor comment changes
*  10/08/2016   R. Repka    Fixed C++ struct warnings 
*  12/03/2016   R. Repka    Removed semicolon from DECLARE_TIMER
*  03/03/2017   R. Repka    Put semicolon from DECLARE_TIMER back in
*  09/01/2017   R. Repka    Always include time.h
******************************************************************************/
#ifndef _TIMERS_H_
#define _TIMERS_H_

/******************************************************************************
* The #warning pre-processor directive provided by the GNU gcc compiler will
* print a warning when the timers are enabled 
******************************************************************************/
#include <stdio.h>
#include <time.h>

/******************************************************************************
* Timers instrumentation macro definitions and use.
* If TIMING is *not* defined the macros are defined as null comments
* Each timer must be declared before use (pay attention to the scope)
* A timer should be stopped before printing, but in case it is printed when
* running, it will be automatically stopped.
* If a timer is not be explicitly started, the starting point will be the same 
* as the declaration
* Previously stopped timers must be started in order to accumulate time values 
* across multiple stop/run sessions, restarting a timer is performed only to
* re-stablish the starting point.
******************************************************************************/
#if defined(EN_TIME)
    struct timerDetails {                                                    
      clock_t Start;    /* Start Time   - set when the timer is started */          
      clock_t Stop;     /* Stop Time    - set when the timer is stopped */                         
      clock_t Elapsed;  /* Elapsed Time - Accumulated when timer is stopped */              
      int State;        /* Timer State  - 0=stopped / 1=running */            
    }; /* Elapsed Time and State must be initialized to zero */            

  /****************************************************************************
  * Declare the timer.  Initialize the timer (particularly the accumulated
  * elapsed time and the timer state).
  * DO NOT USE A SEMICOLON AFTER THE MACRO in your C code, it is defined here
  ****************************************************************************/
  #define DECLARE_REPEAT_VAR(V)       int _t_ ## V;
  
  #define BEGIN_REPEAT_TIMING(R,V)  /* R - repeat value, V - variable */      \
  {                                                                           \
  for(_t_ ## V=0; _t_ ## V < R; _t_ ## V++) {         

  #define END_REPEAT_TIMING  }}

  #define DECLARE_TIMER(A)                                                    \
    struct timerDetails                                                      \
     A = /* Elapsed Time and State must be initialized to zero */             \
      {                                                                       \
      /* Start   = */ 0,                                                      \
      /* Stop    = */ 0,                                                      \
      /* Elapsed = */ 0,                                                      \
      /* State   = */ 0,                                                      \
      }; /* Timer has been declared and defined, ; IS required */

  /****************************************************************************
  * Start the timer.  Print an error if it is already running, set the state 
  * to running and then start the timer.
  ****************************************************************************/
  #define START_TIMER(A)                                                      \
    {                                                                         \
     /* It is an error if the timer is currently running */                   \
     if (1 == A.State) {                                                      \
       fprintf(stderr, "Error, running timer "#A" started.\n");               \
     }                                                                        \
     /* Set the state to running */                                           \
     A.State = 1;                                                             \
     /* Set the start time, done last to maximize accuracy */                 \
     A.Start = clock();                                                       \
    } /* START_TIMER() */

  /****************************************************************************
  * Reset the timer.  Clear the elapsed time.
  ****************************************************************************/
  #define RESET_TIMER(A)                                                      \
    {                                                                         \
     /* Reset the elapsed time to zero */                                     \
     A.Elapsed = 0;                                                           \
    } /* RESET_TIMER() */

  /****************************************************************************
  * Stop the timer.  Set the stop time, print an error message if the timer
  * is already stopped otherwise accumulate the elapsed time (works for
  * both one-time and repeating timing operations), set the state to
  * stopped.
  ****************************************************************************/
  #define STOP_TIMER(A) \
    {                                                                         \
     /* Set the stop time, done first to maximize accuracy */                 \
     A.Stop = clock();                                                        \
     /* It is an error if the timer is currently stopped */                   \
     if (0 == A.State) {                                                      \
       fprintf(stderr, "Error, stopped timer "#A" stopped again.\n");         \
     }                                                                        \
     else { /* accumulate running total only if previously running */         \
       A.Elapsed += A.Stop - A.Start;                                         \
     }                                                                        \
     /* Set the state to stopped */                                           \
     A.State = 0;                                                             \
    } /* STOP_TIMER() */

  /****************************************************************************
  * Print the timer.  Check the timer state and stop it if necessary, print
  * the elapsed time (in seconds).
  ****************************************************************************/
  #define PRINT_TIMER(A)                                                      \
    {                                                                         \
     /* Stop the timer (silently) if it is currently running */               \
     if (1 == A.State) {                                                      \
       STOP_TIMER(A);    /* no error possible in this case */                 \
     }                                                                        \
     fprintf(stderr, "Elapsed CPU Time ("#A") = %g sec.\n",                   \
                     (double)A.Elapsed / (double)CLOCKS_PER_SEC);             \
    } /*PRINT_TIMER() */

  #define PRINT_RTIMER(A,R)                                                   \
    {                                                                         \
     /* Stop the timer (silently) if it is currently running */               \
     if (1 == A.State) {                                                      \
       STOP_TIMER(A);    /* no error possible in this case */                 \
     }                                                                        \
     fprintf(stderr, "Elapsed CPU Time per Iteration ("#A", %d) = % .2e sec.\n",  \
                     R, ((double)A.Elapsed / (double)CLOCKS_PER_SEC)/(double)R);  \
    } /*PRINT_RTIMER() */

#else /* not defined(TIMING) */

  /* Declare null macros for error-free compilation */
  #define DECLARE_TIMER(A)          /* Null Macro */
  #define START_TIMER(A)            /* Null Macro */
  #define RESET_TIMER(A)            /* Null Macro */
  #define STOP_TIMER(A)             /* Null Macro */
  #define PRINT_TIMER(A)            /* Null Macro */
  #define PRINT_RTIMER(A,R)         /* Null Macro */
  #define DECLARE_REPEAT_VAR(R)     /* Null Macro */
  #define BEGIN_REPEAT_TIMING(R,V)  /* Null Macro */
  #define END_REPEAT_TIMING         /* Null Macro */

#endif /* defined(TIMING) */

#endif /* _JCTIMERS_H_ */
