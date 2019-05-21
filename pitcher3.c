//----------------------------------------------------------------------------------------------------------
// PITCHER 3                               												(just videogame mood)
// Just the same stuff of pitcher in order to test what is going on on each thread
//----------------------------------------------------------------------------------------------------------
#include "pnet.h"
#include "pitch.h"
#include "autil.h"
#include "ptask_time.h"
#include "mutils.c"

#define SAMPLE_RATE			44100
#define FRAMES_PER_CHUNK	2205
#define CAPTUR_PERIOD		50.0
#define DIST				(1.0/44100)
#define POWER_THRESH_F32	(0.003)		// IMPORTANT NOTE: this is the threshold we use for recorded files 
										// during training. Hence, we need to tune the microphone in order
										// to get decent comparison of captured signal power


pthread_t capturer;
struct timespec t_cap;

void *capturer_task ()
{
	printf("[CAPTUR] Now active\n");
	
	alsa_param_t	myparams;
	int err;

	// prepare timespec
	t_cap.tv_sec = 0;
	t_cap.tv_nsec = 0;

	// open alsa device for capturing
	capture_handle = alsa_open("default", MODE_CAPT);
	if (capture_handle == NULL)
	{
		fprintf(stderr, "[CAPTUR] capture_handle is NULL\n");
		pthread_exit(NULL);
	}
		
	// we want to acquire numbers that can be easily normalized. Our network has 
	// been trained with values captured in 32bit floating point, so we are going
	// to do the same as well
	alsa_param_init(&myparams); 				// default initialization
	myparams.format = SND_PCM_FORMAT_FLOAT;		
	myparams.frames = FRAMES_PER_CHUNK;
	alsa_param_print(&myparams);
	// harware parameter configuration and freeing
	alsa_hw_param_config(capture_handle, &myparams);
	// prepare
	if ((err = snd_pcm_prepare (capture_handle)) < 0)
	{
		fprintf(stderr, "[CAPTUR] Cannot prepare audio interface for use (%s)\n", snd_strerror(err));
		pthread_exit(NULL);
	}
	printf("[CAPTUR] Ready for capture...\n");
	
	// now, let's test what we capture. Output something just when something 
	// significant is recorded
	// prepare the buffer that resembles what we want to capture
	while (1)
	{
		float tmp_buf[FRAMES_PER_CHUNK];
		if ((err = alsa_capture_float (capture_handle, tmp_buf, &myparams)) < 0)
		{
			if (xrun_recovery(capture_handle, err, CAPTUR_PERIOD) < 0)
			{
				fprintf(stderr, "[CAPTUR] No buffer correctly allocated for capturing\n");
				pthread_exit(NULL);
			}
			// else proceed
		}
		
		// evaluate power of chunk
		float power = euler_trapezoid_float(tmp_buf, DIST, FRAMES_PER_CHUNK);
		if (power >= POWER_THRESH_F32)
			printf("[CAPTUR] Ooh! I'm listening something of %.3f power...\n", power);
		else
		{
			printf("[CAPTUR] Power is only %.3f... I feel deaf :(\r", power);
			fflush(stdout);
		}
		wait_for_period(&t_cap, CAPTUR_PERIOD);
	}
	
	pthread_exit(NULL);
	
}

int main()
{
	int err;
	// crate and launch
	if ((err = pthread_create(&capturer, NULL, capturer_task, NULL)) < 0) 
	{
		fprintf(stderr, "[MAIN] Cannot create capturer thread: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}
	// join
		if ((err = pthread_join(capturer, NULL)) < 0)
	{
		fprintf(stderr, "[MAIN] Cannot join capturer son: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}
	printf("[MAIN] Terminating...");
}


