#ifndef MUTILS_H
#define MUTILS_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

//---------------------------------------------------------------------------
// ADDITIONAL MATH UTILITIES
//---------------------------------------------------------------------------

// degree 1 simple integral (Euler forward)
float 	euler_trapezoid_short (short* values, float h, int n);
float 	euler_trapezoid_int (int* values, float h, int n);
float 	euler_trapezoid_float (float* values, float h, int n);
// evaluate mean of values
float 	mean (float* values, int n);
float 	mean_abs (int* values, int n);
void 	shuffle (int *array, size_t n);

// TESTING a simple shuffling
struct elem
{
	int 			info;
	struct elem* 	next;
};
// shuffle a list
struct elem* shuffle_list (struct elem* head, int dim);

#endif

