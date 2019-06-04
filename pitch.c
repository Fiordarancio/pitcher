#include "pitch.h"

char* which_pitch (int pt)
{
	switch(pt)	
	{
		case A:		return "A";
		case Bb:	return "A#";
		case B:		return "B";
		case C:		return "C";
		case Db:	return "C#";
		case D:		return "D";
		case Eb:	return "D#";
		case E:		return "E";
		case F:		return "F";
		case Gb:	return "F#";
		case G:		return "G";
		case Ab:	return "G#";
		default:	return "";
	}
}

char* which_pitch3 (int pt)
{
	switch(pt)	
	{
		case A3:	return "A3";
		case B3b: 	return "A3#";
		case B3:	return "B3";
		case C3:	return "C3";
		case D3b:	return "C3#";
		case D3:	return "D3";
		case E3b:	return "D3#";
		case E3:	return "E3";
		case F3:	return "F3";
		case G3b:	return "F3#";
		case G3:	return "G3";
		case A3b:	return "G3#";
		case A4:	return "A4";
		case B4b: 	return "A4#";
		case B4:	return "B4";
		case C4:	return "C4";
		case D4b:	return "C4#";
		case D4:	return "D4";
		case E4b:	return "D4#";
		case E4:	return "E4";
		case F4:	return "F4";
		case G4b:	return "F4#";
		case G4:	return "G4";
		case A4b:	return "G4#";
		case A5:	return "A5";
		case B5b: 	return "A5#";
		case B5:	return "B5";
		case C5:	return "C5";
		case D5b:	return "C5#";
		case D5:	return "D5";
		case E5b:	return "D5#";
		case E5:	return "E5";
		case F5:	return "F5";
		case G5b:	return "F5#";
		case G5:	return "G5";
		case A5b:	return "G5#";		
		
		default:	return "";
	}
}

char* which_pitch_verbose (int pt)
{
	switch(pt)	
	{
		case A:		return "A";
		case Bb:	return "Bb/A#";
		case B:		return "B";
		case C:		return "C";
		case Db:	return "Db/C#";
		case D:		return "D";
		case Eb:	return "Eb/D#";
		case E:		return "E";
		case F:		return "F";
		case Gb:	return "Gb/F#";
		case G:		return "G";
		case Ab:	return "Ab/G#";
		default:	return "UNRECOGNIZED PITCH";
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
	printf("Pitch is supposed to be: %s (%d)\n", which_pitch_verbose (pitch_index), pitch_index);
}

void print_winner_pitch3 (float* pts, int n)
{
	// assume dimension is NPITCHES3
	assert (n==NPITCHES3);
	
	int 	i;
	int 	pitch_index = PT3NONDEF;
	float	max = -1;
	for (i=0; i<n; i++)
	{
		if (pts[i] > max)
		{
			pitch_index = i;
			max = pts[i];
		}
		else
			if (pts[i] == max)
				pitch_index = PT3NONDEF;
	}
	printf("Pitch is supposed to be: %s (%d)\n", which_pitch3(pitch_index), pitch_index);
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


