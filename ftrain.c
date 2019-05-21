//------------------------------------------------------------------------------------------------------------
// FFT-BASED TRAINING v2
// Training based on:
//   - pnetlib library (use of struct example properly mounted)
//   - categorical labels
//   - fourier trasformed samples
// 	 - training set built up from sine generator (different pitches with different volumes)
//------------------------------------------------------------------------------------------------------------

#include "pnetlib.h"
#include "pitch.h"
#include "fftw3.h"

#define NHD					8
#define DIM_TSET			1
#define NOISE_STEPS			1

#define SAMPLERATE			44100
#define FRAMES_PER_CHUNK	2205
#define CHANNELS			2 		// must work in this case too
#define MIN_VOLUME			15000

#define BATCHES				10
#define MAX_EPOCHS			20
#define LEARNING_RATE		0.001
#define MOMENTUM			0.001
#define MIN_ERR				0

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

char*	weight_file = "logs/ftrain_wg.txt";		// weights & biases 
char*	prologue_file = "logs/ftrain_pr.txt";	// net structure
char*	lerr_file = "logs/ftrain_locerr.txt";	// local error over batches
char*	gerr_file = "logs/ftrain_glberr.txt";	// global error over epoch

int		pitch_frequencies [] = 
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


//----------------------------------------------------------------------------
// MAIN
//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	printf("FTRAIN 2.0: TRAINING OVER FREQUENCY SAMPLES OF SINUSOIDAL WAVES\n");
	printf("Using FFT on real values, from %d inputs we get %d output values\n\n", FRAMES_PER_CHUNK, (FRAMES_PER_CHUNK/2 + 1));

	// declare everything
	int 			err;
	int 			i, j, f, k, n;
	struct example*	training_set = NULL;
	int				train_size = 0; 						// len of training_set
	p_net*			network;
	int				nhd;									// dynamic #neuron per input and hidden layers
	int 			max_epoch, batches;						// training stuff
	float 			learning_rate, momentum, min_err;
	float			sinwave [FRAMES_PER_CHUNK*CHANNELS];	// fixed arrays for creating training set
	const int 		NUM_INPUTS = (FRAMES_PER_CHUNK/2) + 1;	// effective samples that match with out
	float 			mysampl [NUM_INPUTS];		
	float			mylabel [NPITCHES];
	float 			volume;
	double*			in;
	fftw_complex*	out;
	fftw_plan		p;
	int 			failure_rate;
	float 			freq_noise;								// used for noise on frequency
	
	// get dynamic content
	printf("Neurons for first layer: ");
	if (scanf("%d", &nhd) <= 0)
		nhd = NHD;	
	
	// network creation: create 2 layers (first is input)
	network = p_net_create();
	p_net_init(network, 2); 
	// add hidden in front of input
	add_layer(network, 0, nhd, NUM_INPUTS, sigmoid, ddx_sigmoid);
	// add output layer
	add_layer(network, 1, NPITCHES, nhd, sigmoid, ddx_sigmoid);
	
	print_netinfo(network);
	
	//---------------------------------------------------------------------------
	// Since our sinusoidals are N = FRAMES_PER_CHUNK * CHANNELS, we know that
	// the Fourier transform of them is about M = N//2 + 1. Hence, M is the real
	// size of the samples that we are going to submit to our network.
	//
	// Further step: include some noise into frequency
	//---------------------------------------------------------------------------
	// prepare
	volume = MIN_VOLUME; // base amplitude we can hear quite well from the pc
	in  = (double*) fftw_malloc (sizeof(double) * FRAMES_PER_CHUNK);
	out = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * FRAMES_PER_CHUNK);
	p 	= fftw_plan_dft_r2c_1d(FRAMES_PER_CHUNK, in, out, FFTW_ESTIMATE);
	for (k=0; k<DIM_TSET; k++)
	{
		volume += 30;
		for (f=0; f<NPITCHES; f++)
		{
			freq_noise = -1.0; // added noise goes from -1 to +1 Hz (very low and linear)
			for (n=0; n<NOISE_STEPS; n++)
			{
				freq_noise += n * 2.0/NOISE_STEPS;
				// generate sin
				generate_sin (sinwave, pitch_frequencies[f]+freq_noise, SAMPLERATE, volume, FRAMES_PER_CHUNK, CHANNELS);
				// for each channel, do
				for (j=0; j<CHANNELS; j++)
				{
					// initialize in, then execute plan
					for (i=0; i<FRAMES_PER_CHUNK; i++)
						in[i] = sinwave[i*CHANNELS+j];
					fftw_execute(p);
					// transfer informations into the example: we take the module of the frequency complex
					for (i=0; i<NUM_INPUTS; i++)
						mysampl[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
					for (i=0; i<NPITCHES; i++)
						mylabel[i] = 0.0;
					mylabel[f] = 1.0;
					training_set = insert_example (training_set, mysampl, NUM_INPUTS, mylabel, NPITCHES);
					train_size++;
				}
			}
		}
	}
	// normalization
	normalize_examples(training_set, train_size);
	printf("We got a training set of %d valid couples\n", train_size);
		
	// check that the fft done is good for any of the samples...
/*	FILE* f1 = fopen("logs/ftrain_fft.txt", "a+"); assert(f1!=NULL);*/
/*	FILE* f2 = fopen("logs/ftrain_sine.txt", "a+"); assert(f2!=NULL);*/
/*	printf("We're gonna save the sin/fft of sample with label: ");*/
/*	print_winner_pitch_verbose(training_set->label, training_set->nl);*/
/*	for (i=0; i<NUM_INPUTS; i++)*/
/*		fprintf(f1, "%d,%f\n", i, training_set->samples[i]);*/
/*	for (i=0; i<FRAMES_PER_CHUNK; i++)		*/
/*		fprintf(f2, "%d,%f\n", i, sinwave[i*CHANNELS]);*/
/*	fclose(f1); fclose(f2);*/
		
	//-----------------------------------------------------------------------------------------------
	// TRAINING PHASE
	//-----------------------------------------------------------------------------------------------
	// initialization of parameters
	printf("Insert number of batches: ");
	if (scanf("%d", &batches) <= 0) 
		batches = BATCHES;
	printf("Insert max epoch to reach while training: ");
	if (scanf("%d", &max_epoch) <= 0) 
		max_epoch = MAX_EPOCHS;
	printf("Insert learning rate: ");
	if (scanf("%f", &learning_rate) <= 0)
		learning_rate = LEARNING_RATE;
	printf("Insert momentum: ");
	if (scanf("%f", &momentum) <= 0)
		momentum = MOMENTUM;	
	printf("Insert min error for early stopping: ")	;
	if (scanf("%f", &min_err) <= 0)
		min_err = MIN_ERR;
	printf("\nApplying learning rate %f and momentum %f\n", learning_rate, momentum);
	printf("With %d batches, we have %d samples per batch (surplus: %d)\n", batches, train_size/batches, (train_size%batches));
	printf("We stop at epoch %d or under error %f\n", max_epoch, min_err);
	
	// training algorithm (backprobagation for updating weights)
	printf("Saving error over %s and %s...\n", lerr_file, gerr_file);
	p_net_train_SGD (network, max_epoch, batches, learning_rate, momentum, min_err, &training_set, train_size, lerr_file, gerr_file);
		
	// save results
	printf("Saving weights over %s and %s...\n\n", weight_file, prologue_file);
	if ((err = save_network (network, weight_file, prologue_file)) < 0)
		fprintf(stderr, "FATAL ERROR in saving weights: %s\n", strerror(err));
	
	//---------------------------------------------------------------------------------
	// COUNTER CHECK
	// check the prediction for all the training set! 
	//---------------------------------------------------------------------------------
	failure_rate = 0;
	for (struct example* list=training_set; list!=NULL; list=list->next)
	{
		predict(network, list->samples, list->ns);
/*		printf("Binary prediction\n");*/
/*		get_float_binary_prediction(network, mylabel, NPITCHES, 0.05);*/
/*		print_winner_pitch_verbose (mylabel, NPITCHES);*/
		printf("Full float prediction values\n");
		print_last_prediction(network);
		printf("Winner-takes-all prediction\n");
		get_winner_prediction (network, mylabel, NPITCHES);
		print_winner_pitch_verbose (mylabel, NPITCHES);
		printf("CORRECT ANSWER -> ");
		print_winner_pitch_verbose (list->label, NPITCHES);
		printf("\n");
		if (compare_labels(mylabel, list->label, NPITCHES) < 0)
			failure_rate++;
	}
	printf("Failure rate over training set: %.1f%%\n", ((float)failure_rate)*100/train_size);
		
	// deallocate memory	
	fftw_destroy_plan(p);
	fftw_free(in); fftw_free(out);
	delete_all_examples(training_set);
	p_net_destroy(network);
	return 0;
}


