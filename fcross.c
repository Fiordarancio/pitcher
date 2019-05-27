//-------------------------------------------------------------------------------------------------------------
// CROSS-PITCHER
// In this file, we don't use any network to identifying samples. We generate k samples for each pitch
// and we store them: for each, we first normalize and then we take the FFT transform.
// Once this is done, it is the turn of ALSA-based threads to capture data, normalize and take the FFT.
// The correct pitch is the one which gets the minimum avg squared error taken between all references.
//-------------------------------------------------------------------------------------------------------------
#include "pnetlib.h"
#include "fftw3.h"
#include "pitch.h"

// generate sinusoidal 
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

#define SAMPLERATE			44100
#define FRAMES				2205
#define CHANNELS			2 		// must work in this case too
#define MIN_VOLUME			15000
#define STEP_VOLUME			300

#define BATCHES				10
#define MAX_EPOCHS			20
#define LEARNING_RATE		0.001
#define MOMENTUM			0.001
#define MIN_ERR				0

#define DIM_TSET			5
#define NOISE_STEPS			10

char* norm_file = "logs/fcross_norm.txt";
char* fftw_file = "logs/fcross_fftw.txt";

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
	// variables
	struct example* initial_set = NULL;				// set to be normalized: half is passed to reference
	struct example* reference_set = NULL;
	const int 		NUM_INPUTS = (FRAMES/2) + 1;	// effective samples that match with out
	float 			inisamp [FRAMES];
	float			refsamp [NUM_INPUTS];
	float			label [NPITCHES];
	int 			ini_size = 0;
	int 			ref_size = 0;
	int 			k, n, f, j, i;
	double*			in;
	fftw_complex*	out;
	fftw_plan		p;	
	float 			freq_noise;
	float 			volume;
	float 			sinwave [FRAMES * CHANNELS];
	
	// prepare
	in  = (double*) fftw_malloc (sizeof(double) * FRAMES);
	out = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * FRAMES);
	p 	= fftw_plan_dft_r2c_1d(FRAMES, in, out, FFTW_ESTIMATE);
	
	volume = MIN_VOLUME; // base amplitude we can hear quite well from the pc
	for (k=0; k<DIM_TSET; k++)
	{
		volume += 30;
		for (f=0; f<NPITCHES; f++)
		{
			freq_noise = -2.0; // added noise goes from -2 to +2 Hz (very low and linear)
			for (n=0; n<NOISE_STEPS; n++)
			{
				freq_noise += n * 2.0/NOISE_STEPS;
				// generate sin
				generate_sin (sinwave, pitch_frequencies[f]+freq_noise, SAMPLERATE, volume, FRAMES, CHANNELS);
				// for each channel, insert an example in the initial list, in order to normalize it later
				for (j=0; j<CHANNELS; j++)
				{
					for (i=0; i<FRAMES; i++)
						inisamp[i] = sinwave[i*CHANNELS+j];
					// initialize label just now
					for (i=0; i<NPITCHES; i++)
						label[i] = 0.0;
					label[f] = 1.0;
					initial_set = insert_example (initial_set, inisamp, FRAMES, label, NPITCHES);
					ini_size++;
				}
			}
		}
	}
	// normalization
	normalize_examples(initial_set, ini_size);
	printf("We normalized %d valid sinusoidals over %d channels (size %d)\n", ini_size, CHANNELS, FRAMES);
	
	// fft
	for (struct example* list=initial_set; list!=NULL; list=list->next)
	{
		// initialize in, then execute plan
		for (i=0; i<list->ns; i++)
			in[i] = list->samples[i];
		fftw_execute(p);
		// transfer informations into the example: we take the module of the frequency complex
		for (i=0; i<NUM_INPUTS; i++)
			refsamp[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
		// label is ready, add to reference list
		reference_set = insert_example (reference_set, refsamp, NUM_INPUTS, label, NPITCHES);
		ref_size++;
	}
	printf("The refencence set has %d valid ffts (size %d)\n", ref_size, NUM_INPUTS);	
	
	// let's plot both, so we can check the correctness: take the first
	FILE * f1 = fopen(norm_file, "a+"); assert(f1!=NULL);
	FILE * f2 = fopen(fftw_file, "a+"); assert(f2!=NULL);
	printf("Printing sine (element of initial_set) and fft (correspondent on reference_set) ...\n");
	struct example* exini = get_example(initial_set, 1199);
	struct example* exref = get_example(reference_set, 1199);
	for (i=0; i<FRAMES; i++)
		fprintf(f1, "%d,%f\n", i, exini->samples[i]); //initial_set->samples[i]);
	for (i=0; i<NUM_INPUTS; i++)		
		fprintf(f2, "%d,%f\n", i, exref->samples[i]); //reference_set->samples[i]);
	fclose(f1); fclose(f2);


	// destroy all
	delete_all_examples(reference_set);
	delete_all_examples(initial_set);
	fftw_destroy_plan(p);
	fftw_free(in); fftw_free(out);
	return 0;	
}
