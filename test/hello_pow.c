// FIND THRESHOLD POWER OF SIGNAL
// Read  chunks from a wav file and evaluate energy/power of the signal: the mean of 
// those chunks will be the absolute power threshold to be used for discard or accept 
// a chunk in a training set
// In this test, we observe how computation changes according to data type. We must pay
// attention on which of them we choose when caputirng in ALSA, and also when reading
// wav files for the training set (asserts are needed to be sure)
#include "../wav.h"
#include <math.h>

#define FRAMES_PER_CHUNK	2205 		// frames letti in 50ms
#define PERIOD				(50.0)		// millisec thread period
#define DIST				(1.0/44100)	// temporal distance from sample to sample

// evaluate trapezoidal integral -> THESE FUNCTION ARE PUT INTO mutils.c
float euler_trapezoid_short (short* values, float h, int n)
{
	int 	i;
	float 	sum = 0;
	for (i=0; i<(n-1); i++)
		sum += ((fabs((float)values[i]) + fabs((float)values[i+1])) * h)/2;
	return sum;
}

float euler_trapezoid_int (int* values, float h, int n)
{
	int 	i;
	float 	sum = 0;
	for (i=0; i<(n-1); i++)
		sum += ((fabs((float)values[i]) + fabs((float)values[i+1])) * h)/2;
	return sum;
}

float euler_trapezoid_float (float* values, float h, int n)
{
	int 	i;
	float 	sum = 0;
	for (i=0; i<(n-1); i++)
		sum += ((fabs(values[i]) + fabs(values[i+1])) * h)/2;
	return sum;
}


// evaluate mean of values
float mean (float* values, int n)
{
	float mean = 0;
	for (int i=0; i<n; i++)
		mean += values[i];
	return (mean / n);
}

float mean_fabs (int* values, int n)
{
	float mean = 0;
	for (int i=0; i<n; i++)
		mean += fabs(values[i]);
	return (mean / n);

}

int main(int argc, char** argv)
{
	// filename is given
/*	char* 			basefile = "pitches/Base_power.wav";		// s16*/
/*	char* 			testfile = "pitches/C4ribattuto.wav";*/
	char*			basefile = "pitches/Base_power_32f.wav";	// float 32
	char*			tstfile1 = "pitches_32f/A.wav";
	char*			tstfile2 = "pitches/A4.wav";				// another test on s32
	char*			tstfile3 = "pitches/A4_basepow.wav";		// another test on s16
	
	SNDFILE*		wav_file;
	SF_INFO			wav_info;
	alsa_param_t	params;
	int 			i;
/*	int*			chunk_buffer;*/
	int				totchunks;
	float*			powers;
	float			basemean, localmean;
	int 			accepted=0, discarded=0;	
	
	// open wav
	wav_file = open_wav (basefile, &wav_info, &params);
	if (wav_file == NULL)
	{ 
		fprintf(stderr, "Error while opening file %s\n", basefile);
		exit(EXIT_FAILURE);
	}
	
	// IMPORTANT -> CHECK THE FORMAT OF THE FILE YOU ARE OPENING
/*	assert(params.format == SND_PCM_FORMAT_S16_LE);*/
	assert(params.format == SND_PCM_FORMAT_FLOAT);	
	//-----------------------------------------------------------------------------------------------------------------------
	// EVALUATE MEAN OVER CHUNKS
	//-----------------------------------------------------------------------------------------------------------------------
	// reading the file, we get the total frames registered, given the relative samplerate.
	// We subdivide per FRAMES_PER_CHUNK to obtain the total chunks in it: on each chunk
	// we evaluate the power. In the end, we take the mean of all these powers
	totchunks = (params.frames * params.channels)/FRAMES_PER_CHUNK;
	powers = (float*) malloc (sizeof(float) * totchunks);
	printf("From %d totframes we get %d chunks (surplus %d)\n", params.frames, totchunks, params.frames%FRAMES_PER_CHUNK);
	
	for (i=0; i<totchunks; i++)
	{
		int 	chunkframes = 0;
		float	tmp_chunk [FRAMES_PER_CHUNK * (const int)params.channels];
		while (chunkframes < FRAMES_PER_CHUNK)
		{
			int read_frames = read_wavchunk_float (wav_file, tmp_chunk, FRAMES_PER_CHUNK, params.channels);
			if (read_frames == 0)
				fprintf(stderr, "No other frames to read\n");
			if (read_frames < 0)
			{
				fprintf(stderr, "Fatal error: %s (snd: %s)\n", strerror(read_frames), snd_strerror(read_frames));
				free(powers);
				close_wav(wav_file);
				exit(EXIT_FAILURE);
			}
			chunkframes += read_frames;
		}
		powers[i] = euler_trapezoid_float(tmp_chunk, DIST, FRAMES_PER_CHUNK);
	}
		
	// evaluate power 
	basemean = mean(powers, totchunks);
	printf("Total power of chunks is:   %f\n", basemean*totchunks);
	printf("Mean over all chunks power: %f\n", basemean); 
	
	//-----------------------------------------------------------------------------------------------------------------------
	// EVALUATE MEAN OVER ENTIRE FILE
	//-----------------------------------------------------------------------------------------------------------------------
/*	powers = (float*) malloc (sizeof(float));*/
/*	*/
/*	int 	totframes = 0;*/
/*	short	tmp_chunk [(const int)params.frames * (const int)params.channels];*/
/*	while (totframes < params.frames)*/
/*	{*/
/*		int read_frames = read_wavchunk_short (wav_file, tmp_chunk, params.frames);*/
/*		if (read_frames == 0)*/
/*			fprintf(stderr, "No other frames to read\n");*/
/*		if (read_frames < 0)*/
/*		{*/
/*			fprintf(stderr, "Fatal error: %s (snd: %s)\n", strerror(read_frames), snd_strerror(read_frames));*/
/*			free(powers);*/
/*			close_wav(wav_file);*/
/*			exit(EXIT_FAILURE);*/
/*		}*/
/*		totframes += read_frames;*/
/*	}*/
/*	*powers = euler_trapezoid_short(tmp_chunk, DIST, params.frames*params.channels);*/
/*		*/
/*	// evaluate power */
/*	basemean = *powers / (params.frames*params.channels -1);*/
/*	printf("Total power of chunks is:   %f\n", *powers);*/
/*	printf("Mean over all chunks power: %f\n", basemean); */

	
	free(powers);
	close_wav(wav_file);
	
	// read another file to see how many chunks are accepted or discarded
	wav_file = open_wav (tstfile1, &wav_info, &params);
	if (wav_file == NULL)
	{ 
		fprintf(stderr, "Error while opening file %s\n", tstfile1);
		exit(EXIT_FAILURE);
	}
	
/*	assert(params.format == SND_PCM_FORMAT_S32_LE);*/
	assert(params.format == SND_PCM_FORMAT_FLOAT);
	
	// subdivide file into chunks 
	totchunks = params.frames / FRAMES_PER_CHUNK;
	powers = (float*) malloc (sizeof(float) * totchunks);
	printf("From %d totframes we get %d chunks (surplus %d)\n", params.frames, totchunks, params.frames%FRAMES_PER_CHUNK);
	
	for (i=0; i<totchunks; i++)
	{
		int 	chunkframes = 0;
		float	tmp_chunk [FRAMES_PER_CHUNK*(const int)params.channels];
		while (chunkframes < FRAMES_PER_CHUNK)
		{
			int read_frames = read_wavchunk_float (wav_file, tmp_chunk, FRAMES_PER_CHUNK, params.channels);		
			if (read_frames == 0)
				fprintf(stderr, "No other frames to read\n");
			if (read_frames < 0)
			{
				fprintf(stderr, "Fatal error: %s (snd: %s)\n", strerror(read_frames), snd_strerror(read_frames));
				free(powers);
				close_wav(wav_file);
				exit(EXIT_FAILURE);
			}
			chunkframes += read_frames;
		}
		powers[i] = euler_trapezoid_float(tmp_chunk, DIST, FRAMES_PER_CHUNK);
		if (powers[i] >= basemean)
		{
/*			printf("Power[%d] = %f\n", i, powers[i]);*/
			accepted++;
		}
		else
			discarded++;
	}
	localmean = mean(powers, totchunks);
	printf("Total power of chunks is:   %f\n", localmean*totchunks);
	printf("Mean over all chunks power: %f\n", localmean); 
	printf("Accepted %d and discarded %d chunks\n", accepted, discarded);
	
	free(powers);
	close_wav(wav_file);
	
	//---------------------------------------------------------------------------------------------------------
	// LAST TEST OVER A S32 WAV AND A S16 ONE
	//---------------------------------------------------------------------------------------------------------
	wav_file = open_wav (tstfile2, &wav_info, &params);
	if (wav_file == NULL)
	{ 
		fprintf(stderr, "Error while opening file %s\n", tstfile2);
		exit(EXIT_FAILURE);
	}	
	assert(params.format == SND_PCM_FORMAT_S32_LE);
	
	totchunks = (params.frames * params.channels)/FRAMES_PER_CHUNK;
	powers = (float*) malloc (sizeof(float) * totchunks);
	printf("From %d totframes we get %d chunks (surplus %d)\n", params.frames, totchunks, params.frames%FRAMES_PER_CHUNK);
	
	accepted = 0; discarded = 0;
	for (i=0; i<totchunks; i++)
	{
		int 	chunkframes = 0;
		int		tmp_chunk [FRAMES_PER_CHUNK * (const int)params.channels];
		while (chunkframes < FRAMES_PER_CHUNK)
		{
			int read_frames = read_wavchunk_int (wav_file, tmp_chunk, FRAMES_PER_CHUNK, params.channels);
			if (read_frames == 0)
				fprintf(stderr, "No other frames to read\n");
			if (read_frames < 0)
			{
				fprintf(stderr, "Fatal error: %s (snd: %s)\n", strerror(read_frames), snd_strerror(read_frames));
				free(powers);
				close_wav(wav_file);
				exit(EXIT_FAILURE);
			}
			chunkframes += read_frames;
		}
		powers[i] = euler_trapezoid_int (tmp_chunk, DIST, FRAMES_PER_CHUNK);
		if (powers[i] > basemean)
			accepted++;
		else
			discarded++;
	}
		
	// evaluate power 
	basemean = mean(powers, totchunks);
	printf("Total power of chunks is:   %f\n", basemean*totchunks);
	printf("Mean over all chunks power: %f\n", basemean); 
	printf("Accepted %d and discarded %d chunks\n", accepted, discarded);
		
	free(powers);
	close_wav(wav_file);

	// turn of the 16 one
	wav_file = open_wav (tstfile3, &wav_info, &params);
	if (wav_file == NULL)
	{ 
		fprintf(stderr, "Error while opening file %s\n", tstfile3);
		exit(EXIT_FAILURE);
	}	
	assert(params.format == SND_PCM_FORMAT_S16_LE);
	
	totchunks = (params.frames * params.channels)/FRAMES_PER_CHUNK;
	powers = (float*) malloc (sizeof(float) * totchunks);
	printf("From %d totframes we get %d chunks (surplus %d)\n", params.frames, totchunks, params.frames%FRAMES_PER_CHUNK);
	
	accepted = 0; discarded = 0;
	for (i=0; i<totchunks; i++)
	{
		int 	chunkframes = 0;
		short	tmp_chunk [FRAMES_PER_CHUNK * (const int)params.channels];
		while (chunkframes < FRAMES_PER_CHUNK)
		{
			int read_frames = read_wavchunk_short (wav_file, tmp_chunk, FRAMES_PER_CHUNK, params.channels);
			if (read_frames == 0)
				fprintf(stderr, "No other frames to read\n");
			if (read_frames < 0)
			{
				fprintf(stderr, "Fatal error: %s (snd: %s)\n", strerror(read_frames), snd_strerror(read_frames));
				free(powers);
				close_wav(wav_file);
				exit(EXIT_FAILURE);
			}
			chunkframes += read_frames;
		}
		powers[i] = euler_trapezoid_short (tmp_chunk, DIST, FRAMES_PER_CHUNK);
		if (powers[i] > basemean)
			accepted++;
		else
			discarded++;
	}
		
	// evaluate power 
	basemean = mean(powers, totchunks);
	printf("Total power of chunks is:   %f\n", basemean*totchunks);
	printf("Mean over all chunks power: %f\n", basemean); 
	printf("Accepted %d and discarded %d chunks\n", accepted, discarded);
		
	free(powers);
	close_wav(wav_file);
		
	return 0;
}


