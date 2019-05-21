#include "wav.h"

//-------------------------------------------------------------------------------------
// OPEN AND CLOSE A WAV FILE
//-------------------------------------------------------------------------------------
SNDFILE* open_wav(char* fname, SF_INFO* info, alsa_param_t* params)
{
	SNDFILE * file = sf_open(fname, SFM_READ, info);
	
	if (file == NULL)
		return NULL;
    
    // check it is a wav
    assert((info->format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV);
    // check we use LE
    // assert((info->format & SF_FORMAT_ENDMASK) == SF_ENDIAN_LITTLE);
    
	#ifdef VERBOSE
		printf("Opened file: %s\n", fname);
		printf("    - channels: %d\n", info->channels); 
		printf("    - sample rate: %d\n", info->samplerate);
		printf("    - sections: %d\n", info->sections);		// as our chunks
		printf("    - format: %d (0x%x)\n", info->format, info->format);
		printf("    - total frames: %ld\n", info->frames);
	#endif
    
    // transfer information to parameters to know which function should be
    // used later, depending ont the format
    params->channels 	= info->channels;
	params->samplerate 	= info->samplerate;  
	params->frames 		= info->frames;
	params->chunks 		= info->sections;
	params->format 		= info->format;
	
	// check formats we accept: signed short/int, float, double
	// we assume Little Endianess
	switch((params->format & SF_FORMAT_SUBMASK))
	{
		case SF_FORMAT_PCM_16: // short
			params->format = SND_PCM_FORMAT_S16_LE;
			break;
		case SF_FORMAT_PCM_32: // int
			params->format = SND_PCM_FORMAT_S32_LE;
		 	break;
		case SF_FORMAT_FLOAT:
			params->format = SND_PCM_FORMAT_FLOAT_LE;
			break;
		case SF_FORMAT_DOUBLE:
			params->format = SND_PCM_FORMAT_S16_LE;
			break;
		default:
			fprintf(stderr, "Unrecognized format\n");
			sf_close(file);
			return NULL;
	}
	
    return file;    
}

int close_wav(SNDFILE* file)
{
	int err;
	if ((err = sf_close(file)) < 0)
	{
		fprintf(stderr, "Failed to close wav file\n");
		return err;
	}
	return SF_ERR_NO_ERROR;
}

//-----------------------------------------------------------------------------------------------------------
// READ WAV CHUNKS (of different types)
// This function reads a chunk of N frames and stores it. 
//-----------------------------------------------------------------------------------------------------------
int read_wavchunk_int (SNDFILE* wav_file, int* buffer, int frames, int channels)
{
    int readcount = 0;
    int totcount = 0;

	while(totcount < frames)
	{
		readcount = sf_readf_int (wav_file, buffer + readcount*channels, (frames-readcount));
		if (readcount < 0)
		{
			fprintf(stderr, "Error while reading frames: %s\n", snd_strerror(readcount));
			return readcount;
		}
		totcount += readcount;
	}
/*   	printf("Read %d frames\n", totcount);*/
	return totcount; 
}

// short (S16) version
int read_wavchunk_short (SNDFILE* wav_file, short* buffer, int frames, int channels)
{
    int readcount = 0;
    int totcount = 0;

	while(totcount < frames)
	{
		readcount = sf_readf_short (wav_file, buffer + readcount*channels, (frames-readcount));
		if (readcount < 0)
		{
			fprintf(stderr, "Error while reading frames: %s\n", snd_strerror(readcount));
			return readcount;
		}
		totcount += readcount;
	}
/*   	printf("Read %d frames\n", totcount);*/
	return totcount; 
}

// float version
int read_wavchunk_float (SNDFILE* wav_file, float* buffer, int frames, int channels)
{
    int readcount = 0;
    int totcount = 0;

	while(totcount < frames)
	{
		readcount = sf_readf_float (wav_file, buffer + readcount*channels, (frames-readcount));
		if (readcount < 0)
		{
			fprintf(stderr, "Error while reading frames: %s\n", snd_strerror(readcount));
			return readcount;
		}
		totcount += readcount;
	}
/*   	printf("Read %d frames\n", totcount);*/
	return totcount; 
}

//-----------------------------------------------------------------------------------------------------------
// READ AND PLAY a wav file entirely
//-----------------------------------------------------------------------------------------------------------
// short
int readnplay_wav_short (SNDFILE* wav_file, SF_INFO* wav_info, snd_pcm_t* handle, alsa_param_t* params)
{
	int err;
	short* buffer;
    	
	assert(params->format == SND_PCM_FORMAT_S16_LE);
	
	// prepare handle according to data
	alsa_hw_param_config(handle, params);

   	// allocate space using data
    buffer = (short*) malloc(params->frames * params->channels * sizeof(short));
	
    int readcount = 0;
    // while there is anything ro read
   	printf("Starting read/write loop\n");
    while ((readcount = sf_readf_short(wav_file, buffer, params->frames)) > 0) 
    {
    	if (readcount < params->frames)	
			fprintf(stderr, "Read up to %d frames...\n", readcount);
   		if ((err = alsa_playback(handle, (void*)buffer, params)) < 0)
		{
			fprintf(stderr, "Error during playback\n");
			return err;
		}
 	}
 	// when readcount is zero, we reached end of file
   	printf("End read/write loop\n");
	
	return SF_ERR_NO_ERROR;
}

// int
int readnplay_wav_int (SNDFILE* wav_file, SF_INFO* wav_info, snd_pcm_t* handle, alsa_param_t* params)
{
	int err;
	int* buffer;
    	
	assert(params->format == SND_PCM_FORMAT_S32_LE);
	
	// prepare handle according to data
	alsa_hw_param_config(handle, params);

   	// allocate space using data
    buffer = (int*) malloc(params->frames * params->channels * sizeof(int));
	
    int readcount = 0;
    // while there is anything ro read
   	printf("Starting read/write loop\n");
    while ((readcount = sf_readf_int(wav_file, buffer, params->frames)) > 0) 
    {
    	if (readcount < params->frames)	
			fprintf(stderr, "Read up to %d frames...\n", readcount);
   		if ((err = alsa_playback(handle, (void*)buffer, params)) < 0)
		{
			fprintf(stderr, "Error during playback\n");
			return err;
		}
 	}
 	// when readcount is zero, we reached end of file
   	printf("End read/write loop\n");
	
	return SF_ERR_NO_ERROR;
}

// float
int readnplay_wav_float (SNDFILE* wav_file, SF_INFO* wav_info, snd_pcm_t* handle, alsa_param_t* params)
{
	int err;
	float* buffer;
    	
	assert(params->format == SND_PCM_FORMAT_FLOAT_LE);
	
	// prepare handle according to data
	alsa_hw_param_config(handle, params);

   	// allocate space using data
    buffer = (float*) malloc(params->frames * params->channels * sizeof(float));
	
    int readcount = 0;
    // while there is anything ro read
   	printf("Starting read/write loop\n");
    while ((readcount = sf_readf_float(wav_file, buffer, params->frames)) > 0) 
    {
    	if (readcount < params->frames)	
			fprintf(stderr, "Read up to %d frames...\n", readcount);
   		if ((err = alsa_playback(handle, (void*)buffer, params)) < 0)
		{
			fprintf(stderr, "Error during playback\n");
			return err;
		}
 	}
 	// when readcount is zero, we reached end of file
   	printf("End read/write loop\n");
	
	return SF_ERR_NO_ERROR;
}

// double
int readnplay_wav_double (SNDFILE* wav_file, SF_INFO* wav_info, snd_pcm_t* handle, alsa_param_t* params)
{
	int err;
	double* buffer;
    	
	assert(params->format == SND_PCM_FORMAT_FLOAT64_LE);
	
	// prepare handle according to data
	alsa_hw_param_config(handle, params);

   	// allocate space using data
    buffer = (double*) malloc(params->frames * params->channels * sizeof(double));
	
    int readcount = 0;
    // while there is anything ro read
   	printf("Starting read/write loop\n");
    while ((readcount = sf_readf_double(wav_file, buffer, params->frames)) > 0) 
    {
    	if (readcount < params->frames)	
			fprintf(stderr, "Read up to %d frames...\n", readcount);
   		if ((err = alsa_playback(handle, (void*)buffer, params)) < 0)
		{
			fprintf(stderr, "Error during playback\n");
			return err;
		}
 	}
 	// when readcount is zero, we reached end of file
   	printf("End read/write loop\n");
	
	return SF_ERR_NO_ERROR;
}

