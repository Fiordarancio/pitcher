#include "ptask_time.h"

//------------------------------------------------------------------------------------------
// TIME_COPY: copies a source time variable ts in a destination variable pointed by td
//------------------------------------------------------------------------------------------
void time_copy(struct timespec *td, struct timespec ts)
{
	td->tv_sec  = ts.tv_sec;
	td->tv_nsec = ts.tv_nsec;
}

//--------------------------------------------------------------------------
// TIME_ADD_MS: adds a value ms expressed in milliseconds 
// to the time variable pointed by t
//--------------------------------------------------------------------------
void time_add_ms(struct timespec *t, int ms)
{
	t->tv_sec += ms/MILLI;
	t->tv_nsec += (ms%MILLI)*MICRO;

	if(t->tv_nsec > NANO) {
		t->tv_nsec -= NANO;
		t->tv_sec += 1;
	}
}

//--------------------------------------------------------------------------
// TIME_CMP: compares two time variables t1 and t2 and returns
//  0 if they are equal
//  1 if t1 > t2
// -1 if t1 < t2
//--------------------------------------------------------------------------
int time_cmp(struct timespec t1, struct timespec t2)
{
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}

//--------------------------------------------------------------------------
// TASK METHODS
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// WAIT_FOR_PERIOD: suspends the calling thread until the next activation 
// and, when awaken, updates activation time and deadline
//--------------------------------------------------------------------------
void wait_for_period(struct timespec* tp, int period)
{
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, tp, NULL);
	time_add_ms(tp, period);
}

//--------------------------------------------------------------------------
// SET_PERIOD: reads the current time and computes the next activation 
// time and the absolute deadline of the task
//--------------------------------------------------------------------------
void set_period (struct timespec* tp, int period)
{
	struct timespec t;

	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(tp, t);
	time_add_ms(tp, period);
}



