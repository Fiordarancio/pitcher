// checking the wavs are good enough and that we are correcty used the euler_trapezoid_float
#include "../wav.h"
#include "../mutils.h"

#define FRAMES				2205
#define SAMPLERATE			44100
#define POWER_THRESH_F32	0.0001

int main()
{
	char* filename = "../wav_32f/D4vH.wav";
	SNDFILE*		wav_file = NULL;
	SF_INFO			wav_info;
	alsa_param_t 	wav_params;
	float**			chunk_buffer;	 // array of arrays -> the example samples
	int 			accepted = 0, discarded = 0;
	int 			i, j, f;
	int 			read_frames;
	FILE*			fd;
	
	// open a wav file into wav_32f and observe power
	wav_file = open_wav (filename, &wav_info, &wav_params);
	if (wav_file == NULL)
	{ 
		fprintf(stderr, "Error while opening file %s\n", filename);
		exit(EXIT_FAILURE);
	}
	if (wav_params.format == SND_PCM_FORMAT_S16_LE)
	{
		printf("Warning: %s has 16bit short int format\n", filename);
		close_wav(wav_file);
		exit(EXIT_FAILURE);
	}
	assert(wav_params.format == SND_PCM_FORMAT_FLOAT);	
	// elaborate and save on a file for python
	// remember that aparam->frames contains the whole frames contained into the file
	const int 		totchunks = wav_params.frames / FRAMES;
	float 			chunk_power [totchunks];
	printf("We get %d chunks with a surplus of %d\n", totchunks, (wav_params.frames%FRAMES));

	chunk_buffer = (float**) malloc (sizeof(float*) * totchunks);
	for (i=0; i<totchunks; i++)
	{
		chunk_buffer[i] = (float*) malloc (FRAMES * wav_params.channels * sizeof(float));
		read_frames = read_wavchunk_float (wav_file, chunk_buffer[i], FRAMES, wav_params.channels);
		if (read_frames <= 0)
		{
			fprintf(stderr, "Frames returned: %d\n", read_frames);
			break;
		}
		chunk_power[i] = euler_trapezoid_float(chunk_buffer[i], (1.0/SAMPLERATE), FRAMES);
		printf("Power of chunk %d: %f\n", i, chunk_power[i]);
	}
	// now we have an array of chunks and relative powerness saved
	
	fd = fopen("logs/hello_wav_audio.txt", "a+"); assert(fd!=NULL);
	// cycle on the chunk buffer to select those that have sufficient power. Insert those into list
	// Remeber that for each channel we add a new sample
	for (i=0; i<totchunks; i++)
	{
		if (chunk_power[i] > POWER_THRESH_F32)
		{
			accepted++;				
			if (wav_params.channels > 1)
			{
				for (j=0; j<wav_params.channels; j++)
				{
					float tmp_buf[(const int)FRAMES];
					for (f=0; f<FRAMES; f++)
					{
						tmp_buf[f] = chunk_buffer[i][f*wav_params.channels+j];
						// write on file
						fprintf(fd, "%d,%f\n", (i*FRAMES*wav_params.channels + f*wav_params.channels + j), tmp_buf[f]);
					}
				}
			}
			else
			{
				for (f=0; f<FRAMES; f++)
					fprintf(fd, "%d,%f\n", (i*FRAMES + f), chunk_buffer[i][f]);
			}
		}
		else
			discarded++;
/*		printf("Power of chunk %d is: %f\n", i, chunk_power[i]);*/
	}
	printf("Insert new chunks: %d accepted.", accepted);
	printf("Discarded chunks: %d\n", discarded);
		
	// free memory
	for (i=0; i<totchunks; i++)
		free(chunk_buffer[i]);
	free(chunk_buffer);
		
	return accepted;
	// close wav
	close_wav(wav_file);
	
}
