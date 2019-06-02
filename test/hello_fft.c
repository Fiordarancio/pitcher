//------------------------------------------------------------------------------
// TEST OF GETTING AN FFT FROM THE SAMPLES WE PASS TO FTRAIN
// We build up samples using sin generator: let's verify that these samples
// actually can get trasformed into correct dft using fft23
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "fftw3.h"
#include "../pitch.h"

#define SRATE	44100
#define PERIOD	0.1		// secs
#define FRAMES 	(int)(SRATE*PERIOD)
#define CHANS	2

// returns an array of dim samples plotting a sinusoidal wave. Dimension should have 
// ALREADY been calculated depending to the number of frames and channels we need. 
// This means that dim of array is frames*channels
void generate_sin (float wave [], float frequency, int samplerate, float volume, int frames, int channels)
{
	int i, j;
	unsigned long off = 0;
	for (i=0; i<frames; i++)
	{
		float t = off++ / (float) samplerate;
		float alpha = 2.0 * M_PI * frequency * t;
		float val = volume * sin(alpha);
		for (j=0; j<channels; j++)
			wave[i*channels + j] = val;
	}
}

// normalize each channel separately
void normalize_sin (float wave[], int frames, int channels)
{
	float 	min, max;
	int 	i, j;
	
	for (j=0; j<channels; j++)
	{
		// find min and max
		min = wave[j]; 
		max = wave[j];
		for (i=0; i<frames; i++)
		{
			if (wave[i*channels + j] < min)
			{
				printf("min: %f\n", wave[i*channels + j]);
				min = wave[i*channels + j];
			}
			if (wave[i*channels + j] > max)
			{
				printf("max: %f\n", wave[i*channels + j]);
				max = wave[i*channels + j];
			}
		}
		// normalize
		for (i=0; i<frames; i++)
			wave[i*channels + j] = (wave[i*channels + j] - min)/(max-min);
	}
	
}

int	pitch_frequencies [] = 
{
	261.63, // C4
	277.18, // C#-Db
	293.66, // D
	311.13, // D#-Eb
	329.63, // E
	349.23, // F
	369.99, // F#-Gb
	392.00, // G
	415.30, // G#-Ab
	440.00, // A
	466.17, // A#-Bb
	493.86  // B
}; 

int main()
{
	float 			wave[FRAMES*CHANS];
	char*			fchan [] = {"logs/hello_fft_sinc1.txt", "logs/hello_fft_sinc2.txt" };
	char*			fsin = "logs/hello_fft_sinew.txt";
	FILE*			f [CHANS];
	FILE*			fs;
	double*			in;
	fftw_complex*	out;
	fftw_plan		p;
	int 			i, j;
	int				err;
	
	// open streams
	for (j=0; j<CHANS; j++)
	{
		f[j] = fopen(fchan[j], "a+");
		assert(f[j] != NULL) ;
	}
	fs = fopen(fsin, "a+"); assert(fs!=NULL);
	
	// prepare 
	generate_sin (wave, pitch_frequencies[G], SRATE, 25000, FRAMES, CHANS); // normal volume
	normalize_sin (wave, FRAMES, CHANS); // normalize
	in  = (double*) fftw_malloc (sizeof(double) * FRAMES); // repeat CHANS times
	out = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * FRAMES);
    p 	= fftw_plan_dft_r2c_1d(FRAMES, in, out, FFTW_ESTIMATE);
	
	// note: exploiting channels with a single in vector is wrong, because the plan will consider
	// the channels as sequential values rather than parallel values. We then need to use in 2 
	// times, separately. fftw3 helps us using execute many times
	for (j=0; j<CHANS; j++)
	{
		// init in
		for(i=0; i<FRAMES; i++)
			in[i] = (double) wave [i*CHANS+j];
		// execute plan
		fftw_execute(p);
		// print out of relative channel
		for(i=0; i<(int)((float)FRAMES/2)+1; i++)
			if ((err = fprintf(f[j], "%d,%f,%f\n", i, (float)out[i][0], (float)out[i][1])) < 0)
				fprintf(stderr, "Error while printing channel %d: %s\n", j, strerror(err));
	
	}

	for (i=0; i<FRAMES; i++)
		for (j=0; j<CHANS; j++)
			if ((err = fprintf(fs, "%d,%f\n", (i*CHANS+j), wave[i*CHANS+j])) < 0)
				fprintf(stderr, "Error while printing sine (freq: %s)\n", which_pitch(G));
	
	for(j=0; j<CHANS; j++)
		fclose(f[j]);
	fclose(fs);
	fftw_destroy_plan(p);
	fftw_free(in); fftw_free(out);
}


