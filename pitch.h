#ifndef PITCH_H
#define PITCH_H
//------------------------------------------
// PITCH IDENTIFICATION
//------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

typedef enum {
	PTNONDEF = -1, // following european octave
	C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B 
} pitch; 

#define NPITCHES	12

// say the pitch
char*	which_pitch (int pt);
// give an array of 12 elements, say which pitch wins
void 	print_winner_pitch (float* pts, int n);
void 	print_winner_pitch_verbose (float* pts, int n);
	
#endif

