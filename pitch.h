#ifndef PITCH_H
#define PITCH_H
//------------------------------------------
// PITCH IDENTIFICATION
//------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

// there are 2 references: 1 single octave (addressing
// the main - "central" - one) and 3 octaves (from 3 to 5)
#define NPITCHES	12
typedef enum {
	PTNONDEF = -1, // following european order
	C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B 
} pitch; 

#define NPITCHES3	36
typedef enum {
	PT3NONDEF = -1,
	C3, D3b, D3, E3b, E3, F3, G3b, G3, A3b, A3, B3b, B3,
	C4, D4b, D4, E4b, E4, F4, G4b, G4, A4b, A4, B4b, B4,
	C5, D5b, D5, E5b, E5, F5, G5b, G5, A5b, A5, B5b, B5,	
} pitch3; // 3 octaves 

// the array with frequencies must be defined elsewhere

// say the pitch
char*	which_pitch (int pt);
char* 	which_pitch3 (int pt);
char* 	which_pitch_notation (int pt); // verbose
// give an array of 12/36 elements, say which pitch wins
void 	print_winner_pitch (float* pts, int n);
void	print_winner_pitch3(float* pts, int n);
void 	print_winner_pitch_verbose (float* pts, int n);
	
#endif

