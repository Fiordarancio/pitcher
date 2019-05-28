#ifndef PTASK_H
#define PTASK_H

#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>

#define DEBUG_PRINT_H
#ifdef	DEBUG_PRINT_H
#include <stdio.h>
#include <string.h>
#endif

#define MILLI 		1000
#define MICRO		(MILLI*MILLI)
#define NANO	 	(MILLI*MILLI*MILLI)

//------------------------------------------------------------
// struct timespec {
//	time_t	tv_sec; 	// seconds
//						// time_t = 32b integer
//	long	tv_nsec;	// nanosec
// }
//------------------------------------------------------------

//------------------------------------------------------------
// TIME UTILITIES
//------------------------------------------------------------
void	time_copy (struct timespec* dest, struct timespec src);
// add and subtract time
void 	time_add_s (struct timespec* t, int s);
void 	time_add_ms (struct timespec* t, int ms);
void 	time_add_ns (struct timespec* t, long ns);
void 	time_sub_s (struct timespec* t, int s);
void 	time_sub_ms (struct timespec* t, int ms);
void 	time_sub_ns (struct timespec* t, long ns);
// time comparison
int		time_cmp (struct timespec t1, struct timespec t2);

//------------------------------------------------------------
// TASK METHODS
//------------------------------------------------------------
void 	wait_for_period(struct timespec* tp, int period);
void 	set_period (struct timespec* tp, int period);

#endif

