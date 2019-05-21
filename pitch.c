#include "pitch.h"

char* which_pitch (int pt)
{
	switch(pt)	
	{
		case A:
			return "A";
		case Bb:
			return "Bb/A#";
		case B:
			return "B";
		case C:
			return "C";
		case Db:
			return "Db/C#";
		case D:
			return "D";
		case Eb:
			return "Eb/D#";
		case E:
			return "E";
		case F:
			return "F";
		case Gb:
			return "Gb/F#";
		case G:
			return "G";
		case Ab:
			return "Ab/G#";
		default:
			return "UNRECOGNIZED PITCH";
	}
}

// give an array of 12 elements, say which pitch wins
void print_winner_pitch (float* pts, int n) 
{
	// assume dimension is NPITCHES
	assert (n==NPITCHES);
	
	int 	i;
	int 	pitch_index = PTNONDEF;
	float 	max = -1;
	for (i=0; i<n; i++)
	{
		if (pts[i] > max)
		{
			pitch_index = i;
			max = pts[i];
		}
		else
			if (pts[i] == max)
				pitch_index = PTNONDEF;
	}
	printf("Pitch is supposed to be: %s (%d)\n", which_pitch(pitch_index), pitch_index);
}

// verbose version: print effective values plus the supposition
void print_winner_pitch_verbose (float* pts, int n)
{
	assert (n==NPITCHES);
	
	int 	i;
	printf("[");
	for (i=0; i<n; i++)
		printf(" %s:%.1f ", which_pitch(i), pts[i]);
	printf("] -> ");
	print_winner_pitch(pts, n);
}


