//--------------------------------------------------------------------------------
// FPITCHER: online PITCH recognizER over FFT samples
// In order to distribute load, capturer is responsible of capturing, 
// normalizing and transforming audio samples , while recognizer
// has to issue them to the network and print the prediction
//--------------------------------------------------------------------------------
#include "pnet.h"
#include "autil.h"
#include "pitch.h"
#include "ptask_time.h"
#include "mutils.h"
#include "pnetlib.h"
#include "fftw3.h"

// ids of threads (we use ptask.h elements)
#define CAPTUR_THREAD		0				
#define RECOGN_THREAD		1
// periods are equal to deadlines
#define CAPTUR_PERIOD		50		// ms 
#define RECOGN_PERIOD		50		// ms

#define NLAYERS				3
#define FRAMES_PER_CHUNK	2205	// readable frames in 50 ms
#define THREAD_EXIT_FLAG	(-2)	

#define DIST				(1.0/44100)
#define POWER_THRESH_F32	(0.003)	// averaged on FLOAT -> we need to adapt input!

//--------------------------------------------------------------------------
// A READER-WRITER MUTUAL EXCLUSION PROBLEM
// The capturer task is going to write onto a buffer the chunks it reads
// from interface, while the recognizer thread is going to read it. Under
// the mutual exclusion, we also monitor 2 utility integers, that count 
// the number of times the capturer overwrites the buffer without the 
// recognizer reads the info and proceeses it (it is a latency measure)
//--------------------------------------------------------------------------
pthread_mutex_t	buf_sem;
float*			buffer;						// the object of contention 
int 			overwrite;					// how many times capturer overwrites since last reading
int 			max_overwrite;				// maximum number of overwrites encountered so far
pthread_t		capturer, recognizer;		// thread ids
struct timespec t_cap, t_rec;				// structure for monitoring activation of task

const int 		NUM_INPUTS = FRAMES_PER_CHUNK/2 +1;

void *capturer_task(void* arg)
{
	printf("[CAPTUR] Now active\n");
	
	alsa_param_t	aparam;
	int 			err, i;
	float 			power;
	double*			in;
	fftw_complex*	out;
	fftw_plan 		p;
	float 			tmp_buf [FRAMES_PER_CHUNK];

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
	// IMPORTANT NOTE: we trained the network using single channel samples even if
	// the original examples where multiple channeled. Hence, it's faster to capture
	// always with 1 channel only
	alsa_param_init(&aparam); 				// default initialization
	aparam.format = SND_PCM_FORMAT_FLOAT;		
	aparam.frames = FRAMES_PER_CHUNK;
	aparam.channels = 1;
	alsa_param_print(&aparam);
	// harware parameter configuration and freeing
	alsa_hw_param_config(capture_handle, &aparam);
	// prepare
	if ((err = snd_pcm_prepare (capture_handle)) < 0)
	{
		fprintf(stderr, "[CAPTUR] Cannot prepare audio interface for use (%s)\n", snd_strerror(err));
		pthread_exit(NULL);
	}
	printf("[CAPTUR] Ready for capture...\n");
	
	// according to what just defined, prepare buffers for fourier trasform
	in 	= (double*) fftw_malloc (sizeof(double) * FRAMES_PER_CHUNK);
	out = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * FRAMES_PER_CHUNK);
    p 	= fftw_plan_dft_r2c_1d(FRAMES_PER_CHUNK, in, out, FFTW_ESTIMATE);
	
	// prepare the buffer that resembles what we want to capture
	while (1)
	{
		if ((err = alsa_capture_float(capture_handle, tmp_buf, &aparam)) < 0)
		{
			if (xrun_recovery(capture_handle, err, CAPTUR_PERIOD) < 0)
			{
				fprintf(stderr, "\n[CAPTUR] No buffer correctly allocated for capturing\n");
				pthread_mutex_lock(&buf_sem);
				max_overwrite = THREAD_EXIT_FLAG;
				pthread_mutex_unlock(&buf_sem);
				pthread_exit(NULL);
			}
			// else proceed
		}
		
		// write the buffer with chunks that have decent threshold. Moreover, remember that
		// the number of valuable elements in out are N/2 + 1. The other are just conjugates
		if ((power = euler_trapezoid_float(tmp_buf, DIST, FRAMES_PER_CHUNK)) >= POWER_THRESH_F32)
		{
			// by first, trasform
			for (i=0; i<FRAMES_PER_CHUNK;i++)
				in[i] = (double) tmp_buf[i];
			fftw_execute(p);
			
			// secondly, save results into buffer
			pthread_mutex_lock(&buf_sem);
			for (i=0; i<NUM_INPUTS; i++)
				buffer[i] = (float) sqrt(pow(out[i][0],2) + pow(out[i][1],2)); // NO normalization
			overwrite++;
			if (overwrite > max_overwrite)
				max_overwrite = overwrite;
			if (max_overwrite > 100) // 5 secs
			{
				fprintf(stderr, "\n[CAPTUR] Latency of neural network overcame 5 secs. Closing...\n");
				pthread_mutex_unlock(&buf_sem);
				pthread_exit(NULL);
			}
			pthread_mutex_unlock(&buf_sem);
		}
/*		else */
/*		{*/
/*			printf("[CAPTUR] Power is only %.3f... I feel deaf :(\r", power);*/
/*			fflush(stdout);*/
/*		}*/
		
		wait_for_period(&t_cap, CAPTUR_PERIOD);
	}
	
	pthread_exit(NULL);
}

// questo task crea una rete come quella che abbiamo trainato (si potrebbe mettere un header al file
// in cui mettiamo delle informazioni base della rete per poterla ricorstruire dinamicamente, ma ora
// non e' questo che ci interessa) e carica i pesi salvati nel file 'weights.txt'
void *recognizer_task(void* arg)
{
	printf("[RECOGN] Now active\n");
	
	int 	i;
	p_net* 	network;
	float	predicted_label [NPITCHES];
	float	tmp_buf [NUM_INPUTS];
	int 	oflag = 0; // do the computation once
	
	// load the network
	network = load_network("logs/ftrain_wg.txt", "logs/ftrain_pr.txt");
	print_netinfo(network);

	// take mutual exclusion for reading the buffer and give it to the network
	while(1)
	{
		pthread_mutex_lock(&buf_sem);
		// check the buffer is not empty		
		if (overwrite > -1)
		{
			if (max_overwrite > 100)
			{
				fprintf(stderr, "\n[RECOGN] Latency of neural network overcame 5 secs. Closing...\n");
				pthread_mutex_unlock(&buf_sem);
				p_net_destroy(network);
				pthread_exit(NULL);
			}
			if (max_overwrite == THREAD_EXIT_FLAG)
			{
				fprintf(stderr, "\n[RECOGN] No valid buffers available due to ALSA errors. Closing...\n");
				pthread_mutex_unlock(&buf_sem);
				p_net_destroy(network);
				pthread_exit(NULL);
			}
			
			// read the buffer
			for (i=0; i<NUM_INPUTS; i++)
				tmp_buf[i] = buffer[i];
			overwrite = -1;
			oflag = 1;
		}
		else 
			oflag = 0;
		pthread_mutex_unlock(&buf_sem);

		if (oflag > 0)
		{
			dbg_printf("[RECOGN] Computing...\n");
			// do the computation
			predict(network, tmp_buf, NUM_INPUTS);
			get_winner_prediction (network, predicted_label, NPITCHES);
			#ifdef __PNET_DEBUG__
			dbg_printf("Prediction = [");
			for (i=0; i<NPITCHES; i++)
				dbg_printf(" %f ", predicted_label[i]);
			dbg_printf("]\n");
			#endif
			print_winner_pitch (predicted_label, NPITCHES);
		}
		// this thread should not stop for period, but keeps running -> it's slower
		wait_for_period(&t_rec, RECOGN_PERIOD);
	}
	pthread_exit(NULL);
}


int main() {

	int	err;	
	
	// initialize buffer: it is big as NUM_INPUTS
	buffer = (float*) malloc (sizeof(float) * NUM_INPUTS);
	// initialize mutex
	if ((err = pthread_mutex_init (&buf_sem, NULL)) < 0)
	{
		fprintf(stderr, "[MAIN] Error while initializing semaphore: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}
	overwrite = -1;
	max_overwrite = -1;
	// set capturer period
	set_period(&t_cap, CAPTUR_PERIOD);
	set_period(&t_rec, RECOGN_PERIOD);

	// crate and launch both threads
	if ((err = pthread_create(&capturer, NULL, capturer_task, NULL)) < 0) 
	{
		fprintf(stderr, "[MAIN] Cannot create capturer thread: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = pthread_create(&recognizer, NULL, recognizer_task, NULL)) < 0) 
	{
		fprintf(stderr, "[MAIN] Cannot create recognizer thread: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}

	// wait for them
	if ((err = pthread_join(capturer, NULL)) < 0)
	{
		fprintf(stderr, "[MAIN] Cannot join capturer son: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = pthread_join(recognizer, NULL)) < 0)
	{
		fprintf(stderr, "[MAIN] Cannot join recognizer son: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}

	free(buffer);
	printf("[MAIN] Terminating...\n");
	return 0;
}

