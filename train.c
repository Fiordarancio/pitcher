//------------------------------------------------------------------------------------------------------------
// SAMPLE-BASED TRAINING v2
// Training based on:
//   - pnetlib library (use of struct example properly mounted)
//   - categorical labels
// 	 - training set built up from sine generator (different pitches with different volumes)
//------------------------------------------------------------------------------------------------------------

#include "pnetlib.h"
#include "pitch.h"
#include "autil.h"

#define NHD					8
#define DIM_TSET			10

#define SAMPLERATE			44100
#define FRAMES_PER_CHUNK	2205
#define CHANNELS			2 		// must work in this case too

#define BIAS				0
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
/*	float* wave = (float*) malloc (sizeof(float) * frames * channels);*/
	unsigned long off = 0;
	for (i=0; i<frames; i++)
	{
		float t = off++ / (float) samplerate;
		float alpha = 2.0 * M_PI * frequency * t;
		float val = volume * sin(alpha);
		for (j=0; j<channels; j++)
			wave[i*channels + j] = val;
	}
/*	return wave;*/
}

char*	weight_file = "logs/ftrain_wg.txt";
char*	prologue_file = "logs/ftrain_pr.txt";
char*	lerr_file = "logs/ftrain_locerr.txt"; // local error
char*	gerr_file = "logs/ftrain_glberr.txt"; // local error

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

	// declare everything
	int 			err;
	int 			i, f, k;
	struct example*	training_set = NULL;
	int				train_size = 0; 						// len of training_set
	p_net*			network;
	int				nhd;									// dynamic #neuron per input and hidden layers
	float			bias;
	int 			max_epoch, batches;						// training stuff
	float 			learning_rate, momentum, min_err;
	float			mysampl [FRAMES_PER_CHUNK*CHANNELS];	// fixed arrays for creating training set
	float			mylabel [NPITCHES];
	float 			volume;
	
	// get dynamic content
	printf("Neurons for first layer: ");
	if (scanf("%d", &nhd) <= 0)
		nhd = NHD;	
	printf("Initial bias : ");
	if (scanf("%f", &bias) <= 0)
		bias = BIAS;	
	
	// network creation: create 2 layers (first is input)
	network = p_net_create();
	p_net_init(network, 2); 
	// add hidden in front of input
	add_layer(network, 0, nhd, FRAMES_PER_CHUNK*CHANNELS, bias, sigmoid, ddx_sigmoid);
	// add output layer
	add_layer(network, 1, NPITCHES, nhd, bias, sigmoid, ddx_sigmoid);
	
	print_netinfo(network);
	
	//---------------------------------------------------------------------------
	// Create a training set composed by examples that have a number of samples
	// equal to FRAMES_PER_CHUNK * CHANNELS and a label of NPITCHES. Volume goes
	// uniformly from 22000 to 22000+(FRAMES_PER_CHUNK*CHANNELS) 
	volume = 22000;
	for (k=0; k<DIM_TSET; k++)
	{
		volume += 30;
		for (f=0; f<NPITCHES; f++)
		{
			generate_sin (mysampl, pitch_frequencies[f], SAMPLERATE, volume, FRAMES_PER_CHUNK, CHANNELS);
			for (i=0; i<NPITCHES; i++)
				mylabel[i] = 0.0;
			mylabel[f] = 1.0;
			training_set = insert_example (training_set, mysampl, FRAMES_PER_CHUNK*CHANNELS, mylabel, NPITCHES);
			train_size++;
		}
	}
	printf("We got a training set of %d valid couples\n", train_size);
	
	// suoniamo il training set per essere sicuri che sia tutto ok? 
/*	playback_handle = NULL;*/
/*	playback_device = "default";*/
/*	alsa_param_t param;*/
/*	param.samplerate 	= SAMPLERATE;*/
/*	param.channels 		= CHANNELS;*/
/*	param.frames		= FRAMES_PER_CHUNK;*/
/*	param.chunks		= 1;*/
/*	param.format		= SND_PCM_FORMAT_FLOAT;*/
/*	playback_handle = alsa_open (playback_device, MODE_PLAY);*/
/*	if ((alsa_hw_param_config (playback_handle, &param)) < 0)*/
/*	{*/
/*		fprintf(stderr, "Alsa error occurred\n");*/
/*		goto end;*/
/*	}*/
/*	for (k=0; k<train_size; k++)*/
/*	{*/
/*		struct example* ex = get_example(training_set, k);*/
/*		alsa_playback(playback_handle, ex->samples, &param);*/
/*	}*/
	// Boh, sembra fare
	
/*	// train_size counts the dimension of the training set*/
/*	normalize_tset(training_set, train_size, FRAMES_PER_CHUNK);*/

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
	printf("\nApplying initial bias %f, learning rate %f and momentum %f\n", bias, learning_rate, momentum);
	printf("With %d batches, we have %d samples per batch (surplus: %d)\n", batches, train_size/batches, (train_size%batches));
	printf("We stop at epoch %d or under error %f\n", max_epoch, min_err);
	
	// training algorithm (backprobagation for updating weights)
	printf("Saving error over %s and %s...\n", lerr_file, gerr_file);
	p_net_train_SGD (network, max_epoch, batches, learning_rate, momentum, min_err, &training_set, train_size, lerr_file, gerr_file);
		
	// save results
	printf("Saving weights over %s and %s...\n", weight_file, prologue_file);
	if ((err = save_network (network, weight_file, prologue_file)) < 0)
		fprintf(stderr, "FATAL ERROR in saving weights: %s\n", strerror(err));
	
	//---------------------------------------------------------------------------------
	// COUNTER CHECK
	// check the prediction for all the training set! 
	//---------------------------------------------------------------------------------
	for (struct example* list=training_set; list!=NULL; list=list->next)
	{
		predict(network, list->samples, list->ns);
		get_float_binary_prediction(network, mylabel, NPITCHES);
		print_winner_pitch_verbose (mylabel, NPITCHES);
		printf("CORRECT ANSWER\t");
		print_winner_pitch_verbose (list->label, NPITCHES);
	}	
		
/*	end:*/
/*	alsa_close(playback_handle);*/
	// deallocate memory	
	delete_all_examples(training_set);
	p_net_destroy(network);
	return 0;
}


