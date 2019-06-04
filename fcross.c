//-------------------------------------------------------------------------------------------------------------
// CROSS-PITCHER
// In this file, we don't use any network to identifying samples. We generate k samples for each pitch
// and we store them: for each, we first normalize and then we take the FFT transform.
// Once this is done, it is the turn of ALSA-based threads to capture data, normalize and take the FFT.
// The correct pitch is the one which gets the minimum avg squared error taken between all references.
//-------------------------------------------------------------------------------------------------------------
#include "autil.h"
#include "fftw3.h"
#include "pitch.h"
#include "pnetlib.h"
#include "ptask_time.h"
#include "mutils.h"

#define SAMPLERATE			44100
#define FRAMES				2205
#define CHANNELS			1 		// must work in this case too
#define MIN_VOLUME			15000
#define STEP_VOLUME			300

#define DIM_TSET			1
#define NOISE_STEPS			10

#define CAPTUR_PERIOD		50 		// ms
#define POWER_THRESH_F32	(0.003)

char* norm_file = "logs/fcross_norm.txt";
char* fftw_file = "logs/fcross_fftw.txt";

float	pitch_frequencies [] = 
{
	130.81, // C3
	138.59, // C3#-D4b
	146.83, // D3
	155.56, // D3#-E3b
	164.81, // E3
	174.61, // F3
	185.00, // F3#-G3b
	196.00, // G3
	207.65, // G3#-A3b
	220.00, // A3
	233.08, // A3#-B3b
	246.94, // B3
	261.63, // C4
	277.18, // C4#-D4b
	293.66, // D4
	311.13, // D4#-E4b
	329.63, // E4
	349.23, // F4
	369.99, // F4#-G4b
	392.00, // G4
	415.30, // G4#-A4b
	440.00, // A4
	466.17, // A4#-B4b
	493.88, // B4
	523.25, // C5
	554.37, // C5#-D5b
	587.33, // D5
	622.25, // D5#-E5b
	659.26, // E5
	698.46, // F5
	739.99, // F5#-G5b
	783.99, // G5
	830.61, // G5#-A5b
	880.00, // A5
	932.33, // A5#-B5b
	987.77  // B5
}; 

pthread_t				capturer;			
struct timespec 		t_cap;	// structure for monitoring activation of task

// task argument used to pass objects to a thread
struct task_parameter
{
	struct example* set;
	int 			size;
	int 			samp_size; // NUM_INPUTS
};

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

// normalize an array of real numbers
void normalize (float array[], int dim)
{
	float 	min, max;
	int 	i;
	// search max and min
	min = array[0];
	max = array[0];
	for (i=0; i<dim; i++)
	{
		if (array[i] < min)
			min = array[i];
		if (array[i] > max)
			max = array[i];
	}
	// apply formula for all samples
	for (i=0; i<dim; i++)
		array[i] = (array[i] - min) / (max - min);

}

//--------------------------------------------------------------------------------------------------------
// THREADS
//--------------------------------------------------------------------------------------------------------
void *capturer_task(void* arg) 
{
	printf("[CAPTUR] Now active\n");
	
	alsa_param_t			a_param;
	int 					err, i, j;
	double*					in;
	fftw_complex*			out;
	fftw_plan 				p;
	struct task_parameter*	t_param;
	float 					power;
	float 					tmp_buf[FRAMES*CHANNELS];

	// get reference_set and its size through a param 
	t_param = (struct task_parameter*) arg;
	assert (t_param->set != NULL);
	// open alsa device for capturing
	capture_handle = alsa_open("default", MODE_CAPT);
	if (capture_handle == NULL)
	{
		fprintf(stderr, "[CAPTUR] capture_handle is NULL\n");
		pthread_exit(NULL);
	}
		
	// we want to acquire numbers that can be easily normalized
	alsa_param_init(&a_param); 				
	a_param.format = SND_PCM_FORMAT_FLOAT;		
	a_param.frames = FRAMES;
	a_param.channels = CHANNELS;
	alsa_param_print(&a_param);
	// harware parameter configuration and freeing
	alsa_hw_param_config(capture_handle, &a_param);
	// prepare
	if ((err = snd_pcm_prepare (capture_handle)) < 0)
	{
		fprintf(stderr, "[CAPTUR] Cannot prepare audio interface for use (%s)\n", snd_strerror(err));
		pthread_exit(NULL);
	}
	printf("[CAPTUR] Ready for capture...\n");
	
	// according to what just defined, prepare buffers for fourier trasform. We tranform one channel 
	// at a time, so we could get more than one pitch over different channels!
	in 	= (double*) fftw_malloc (sizeof(double) * FRAMES);
	out = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * FRAMES);
    p 	= fftw_plan_dft_r2c_1d(FRAMES, in, out, FFTW_ESTIMATE);
	
	// prepare the buffer that resembles what we want to capture
	do
	{
		if ((err = alsa_capture_float(capture_handle, tmp_buf, &a_param)) < 0)
		{
			if (xrun_recovery(capture_handle, err, CAPTUR_PERIOD) < 0)
			{
				fprintf(stderr, "\n[CAPTUR] No buffer correctly allocated for capturing\n");
				pthread_exit(NULL);
			}
			// else proceed
		}
		
		// check the power 
		if ((power = euler_trapezoid_float (tmp_buf, 1.0/SAMPLERATE, FRAMES)) >= POWER_THRESH_F32)
		{
			// normalize what we got
			normalize (tmp_buf, FRAMES*CHANNELS);
			
			// DEBUG *** write it on a file!!
			FILE* ff = fopen("logs/capturer_norm.txt", "a+"); assert(ff!=NULL);
			for (i=0; i<FRAMES; i++)
				fprintf(ff, "%d,%f\n", i,tmp_buf[i*CHANNELS]);
			fclose(ff);
			
			// do the fft
			for (j=0; j<CHANNELS; j++)	
			{
				for (i=0; i<FRAMES; i++)
					in[i] = (double) tmp_buf[i*CHANNELS + j];
				fftw_execute(p);
				
				// DEBUG *** write it on a file!!
				FILE* ff = fopen("logs/capturer_fft.txt", "a+"); assert(ff!=NULL);
				for (i=0; i<t_param->samp_size; i++)
				{
					float module = sqrt(pow((float)out[i][0],2) + pow((float)out[i][1],2));
					fprintf(ff, "%d,%f\n", i,module);
				}
				fclose(ff);
		
				// for each of the references, do the avg squared error with the samples and out:
				// The minimum error let us save the relative label
				float 	avg_error = 0;
				float	min_error = RAND_MAX;
				int		min_pitch = PTNONDEF;
				int 	k=0;
				min_error = RAND_MAX; // enormous number
				for (struct example* list=t_param->set; list!=NULL; list=list->next)
				{
					assert(list->ns == t_param->samp_size);
					avg_error = 0;
					for (i=0; i<list->ns; i++)
					{
						float module = sqrt(pow((float)out[i][0],2) + pow((float)out[i][1],2));
						avg_error += pow((module - list->samples[i]), 2);
					}
					avg_error = sqrt(avg_error/list->ns);
					#ifdef __PNET_DEBUG__
					dbg_printf("Sample %d avg_err: %f; lab [", k, avg_error);
					for (i=0; i<NPITCHES3; i++)
						dbg_printf(" %d ", (int)list->label[i]);
					dbg_printf("]\n");
					#endif
					if (avg_error < min_error)
					{
						min_error = avg_error;
						for (i=0; i<NPITCHES3; i++)
							if (list->label[i] == 1.0)
								min_pitch = i;
/*						assert(i<NPITCHES);*/
/*						min_pitch = i;*/
					}
					k++;
				}
				dbg_printf("avg err: %f, min avg: %f, min_pitch: %d\n", avg_error, min_error, min_pitch);
				if (CHANNELS > 1)
					printf("On channel %d pitch is %s\n", j, which_pitch3 (min_pitch));
				else
					printf("I suppose you're playing pitch %s\n", which_pitch3 (min_pitch));
			} 
			dbg_printf("\n");
		}
		else
		{
			printf("I don't hear anything... :(\r");
			fflush(stdout);
		}
		wait_for_period(&t_cap, CAPTUR_PERIOD);
	} while (1);
	
	printf("[CAPTUR] Thread is exiting...\n");
	// delete everything
	fftw_destroy_plan(p);
	fftw_free(in); fftw_free(out);
	pthread_exit(NULL);
}



//--------------------------------------------------------------------------------------------------------
// MAIN
//--------------------------------------------------------------------------------------------------------
int main() 
{
	printf("[MAIN] Setting up program...\n");

	// variables
	struct example* 		initial_set = NULL;				// set to be normalized: half is passed to reference
	struct example* 		reference_set = NULL;
	const int		 		NUM_INPUTS = (FRAMES/2) + 1;	// effective samples that match with out
	float 					inisamp [FRAMES];
	float					refsamp [NUM_INPUTS];
	float					label [NPITCHES3];
	int 					ini_size = 0;
	int 					ref_size = 0;
	int 					k, n, f, j, i;
	int 					err;
	double*					in;
	fftw_complex*			out;
	fftw_plan				p;	
	float		 			freq_noise = 0;
	float		 			volume;
	float 					sinwave [FRAMES * CHANNELS];
	struct task_parameter 	passed_param;

	
	// prepare in, out and plan: notice we see a channel at a time
	in  = (double*) fftw_malloc (sizeof(double) * FRAMES);
	out = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * FRAMES);
	p 	= fftw_plan_dft_r2c_1d(FRAMES, in, out, FFTW_ESTIMATE);
	
	volume = MIN_VOLUME; // base amplitude we can hear quite well from the pc
	for (k=0; k<DIM_TSET; k++)
	{
		volume += 30;
		for (f=0; f<NPITCHES3; f++)
		{
			freq_noise = -1.0; // added noise goes from -1 to +1 Hz (very low and linear)
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
					for (i=0; i<NPITCHES3; i++)
						label[i] = 0.0;
					label[f] = 1.0;
					#ifdef __PNET_DEBUG__
					dbg_printf("f (pitch index) is now: %d -> label[", f);
					for (i=0; i<NPITCHES3;i++)
						dbg_printf(" %d ", (int)label[i]);
					dbg_printf("]\n");
					#endif
					initial_set = insert_example (initial_set, inisamp, FRAMES, label, NPITCHES3);
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
		#ifdef __PNET_DEBUG__
		dbg_printf("Label [");
		for (i=0; i<NPITCHES3; i++)
			dbg_printf(" %d ", (int)list->label[i]);
		dbg_printf("]\n");
		#endif
		reference_set = insert_example (reference_set, refsamp, NUM_INPUTS, list->label, NPITCHES3);
		ref_size++;
	}
	printf("The reference set has %d valid ffts (size %d)\n", ref_size, NUM_INPUTS);	

	#ifdef __PNET_DEBUG__	
	// let's plot both, so we can check the correctness: take the first
	FILE * f1 = fopen(norm_file, "a+"); assert(f1!=NULL);
	FILE * f2 = fopen(fftw_file, "a+"); assert(f2!=NULL);
	printf("Printing sine (element of initial_set) and fft (correspondent on reference_set) ...\n");
	for (i=0; i<FRAMES; i++)
		fprintf(f1, "%d,%f\n", i, initial_set->samples[i]);
	for (i=0; i<NUM_INPUTS; i++)		
		fprintf(f2, "%d,%f\n", i, reference_set->samples[i]);
	fclose(f1); fclose(f2);
	#endif

	//---------------------------------------------------------------------------------------------------
	// ok, now we start sampling as we didi into pitcher. For each sampled chunk of FRAMES samples and 
	// CHANNELS channels, we normalize as before and we compare the fft signals using average squared 
	// error. The index with minimum error is the one we want, so we get the label.
	// We delegate this work to a periodic thread
	//---------------------------------------------------------------------------------------------------	
	// set capturer period
	set_period(&t_cap, CAPTUR_PERIOD);
	// set the parameter to pass
	passed_param.set  = reference_set;
	passed_param.size = ref_size;
	passed_param.samp_size = NUM_INPUTS;
	// crate and launch both threads
	if ((err = pthread_create(&capturer, NULL, capturer_task, (void*) &passed_param)) < 0) 
	{
		fprintf(stderr, "[MAIN] Cannot create capturer thread: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}
	// wait for the end
	pthread_join(capturer, NULL);
	printf("[MAIN] Thread termined. Exiting...\n");
	
	// destroy all
	delete_all_examples(reference_set);
	delete_all_examples(initial_set);
	fftw_destroy_plan(p);
	fftw_free(in); fftw_free(out);
	return 0;	
}
