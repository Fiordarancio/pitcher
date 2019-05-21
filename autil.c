#define AUTIL_H
#include "autil.h"

//---------------------------------------------------------------------------
// OPEN & CLOSE HANDLE
//---------------------------------------------------------------------------
snd_pcm_t* alsa_open (char* device, int mode)
{
	int err;
	snd_pcm_t* handle;
	
	if (mode == MODE_CAPT)
	{
		if ((err = snd_pcm_open (&handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) 
		{
			fprintf (stderr, "cannot open audio device %s (%s)\n", device, snd_strerror (err));
			return NULL;
		}
		return handle;
	}
	
	if (mode == MODE_PLAY)
	{
		if ((err = snd_pcm_open (&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
		{
			fprintf (stderr, "cannot open audio device %s (%s)\n", device, snd_strerror (err));
			return NULL;
		}
		return handle;
	}
	
	fprintf(stderr, "Invalid mode\n");
	return NULL;
}

int alsa_close(snd_pcm_t* handle)
{
	int err;
	if ((err = snd_pcm_close(handle)) < 0)
	{
		fprintf(stderr, "Error while closing/freeing PCM resources: %s\n", snd_strerror(err));
		return err;
	}
	return 0;
}

//---------------------------------------------------------------------------
// BASIC PARAMETER CONFIGURATION
//---------------------------------------------------------------------------
void alsa_param_init(alsa_param_t* aparams)
{
	assert (aparams != NULL);
	aparams->samplerate = DEF_SAMPLERATE;
	aparams->channels 	= DEF_CHANNELS;
	aparams->frames 	= DEF_FRAMES;
	aparams->chunks 	= DEF_CHUNKS;
	aparams->format		= DEF_FORMAT;
}

void alsa_param_print(alsa_param_t* aparams)
{
	assert (aparams != NULL);
	printf("ALSA PARAMS:\n");
	printf("  samplerate: %u\n", aparams->samplerate);	
	printf("  channels: %u\n", aparams->channels);
	printf("  frames: %u\n", aparams->frames);
	printf("  chunks: %u\n", aparams->chunks);
	printf("  format: %u (0x%x)\n", aparams->format, aparams->format);
}

//---------------------------------------------------------------------------
// HARDWARE PARAMETERS CONFIGURATION 
//---------------------------------------------------------------------------
int alsa_hw_param_config (snd_pcm_t* handle, alsa_param_t* aparams)
{
	int err;
	snd_pcm_hw_params_t* hw_params;
	
	// config
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
	{
		fprintf (stderr, "Cannot allocate hw parameter structure (%s)\n", snd_strerror (err));
		return err;
	}
	if ((err = snd_pcm_hw_params_any(handle, hw_params)) < 0)
	{
		fprintf(stderr, "Cannot initialize hw parameter structure: %s\n", snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_hw_params_set_access (handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf(stderr, "Cannot set access type: %s\n", snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_hw_params_set_format (handle, hw_params, aparams->format)) < 0)
	{ 
		fprintf(stderr, "Cannot set sample format: %s\n", snd_strerror(err));
		return err;
	}
	unsigned int approx_rate = aparams->samplerate;
	if ((err = snd_pcm_hw_params_set_rate_near(handle, hw_params, &approx_rate, 0)) < 0)
	{
		fprintf(stderr, "Cannot set sample rate to %u: %s\n", approx_rate, snd_strerror(err));
		return err;
	}
	if (approx_rate != aparams->samplerate) // errors on sampling rate not accepted
	{
		fprintf(stderr, "Rate doesn't match (requested %d Hz, got %uHz)\n", aparams->samplerate, approx_rate);
		return -EINVAL;
	}
	printf("Got sample rate: %u\n", approx_rate);
	
	if ((err = snd_pcm_hw_params_set_channels(handle, hw_params, aparams->channels)) < 0)
	{
		fprintf(stderr, "Cannot set channel count to %d: %s\n", aparams->channels, snd_strerror(err));
		return err;
	
	}
	// end of configuration of hw parameters: set them and free memory

	if ((err = snd_pcm_hw_params(handle, hw_params)) < 0)
	{
		fprintf(stderr, "Cannot set hw parameters: %s\n", snd_strerror(err));
		return err;
	}

	snd_pcm_hw_params_free(hw_params);
	return 0;
}
//----------------------------------------------------------------------------------------------
// XRUN RECOVERY (UNDERRUN/OVERRUN)
// In both cases, the interface was not fast enough to deliver information from/to the device. 
// An overrun occurs when the audio device cannot provide data to process quickly enough.
/// Usually, it happens when you run at very low buffer size while the sound card should process 
// incoming buffers very fast.
// An underrun occurs when the application can't pass data to the audio buffer quickly enough.
//----------------------------------------------------------------------------------------------
int xrun_recovery(snd_pcm_t *handle, int err, unsigned long usec)
{
	if (err == -EPIPE) 			// underrun
	{
		err = snd_pcm_prepare(handle);
		if (err < 0)
			fprintf(stderr, "Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
/*		return 0; 				// why does it return success?*/
		return err;
	} 
	else if (err == -ESTRPIPE) 	// stream pipe error
	{
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
		{
			fprintf(stderr, "Try again after a dummy wait of %ld usec\n", usec);
			usleep(usec); 		// wait until the suspend flag is released
		}
		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0)
				fprintf(stderr, "Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
/*			printf("Preparing after ESTRPIPE: %d\n", err);*/
		}
/*		printf("Preparing finally after ESTRIPE: %d\n", err);*/
		return err; 							
	}
	fprintf(stderr, "Unknown error code %d\n", err);
	return err; 				// it is another unknown error
}


//---------------------------------------------------------------------------
// CAPTURE INTERLEAVED FRAMES OF A CHUNK (preparation must be done outside)
//---------------------------------------------------------------------------
int alsa_capture_float (snd_pcm_t* handle, float* buffer, alsa_param_t* aparams)
{
	int 	readcount = 0;
	int		totcount = 0;
	while (totcount < aparams->frames)
	{
		readcount = snd_pcm_readi (handle, buffer + readcount*aparams->channels, (aparams->frames-readcount));
		if (readcount < 0)
		{
			fprintf(stderr, "Failed to read from audio interface: %s\n", snd_strerror(readcount));
			return readcount;
		}
		totcount += readcount;
	}
	return totcount;	
}

// int version
int alsa_capture_int (snd_pcm_t* handle, int* buffer, alsa_param_t* aparams)
{
	int 	readcount = 0;
	int		totcount = 0;
	while (totcount < aparams->frames)
	{
		readcount = snd_pcm_readi (capture_handle, buffer+readcount*aparams->channels, aparams->frames-readcount);
		if (readcount < 0)
		{
			fprintf(stderr, "Failed to read from audio interface: %s\n", snd_strerror(readcount));
			return readcount;
		}
		totcount += readcount;
	}
	return totcount;	
}

//---------------------------------------------------------------------------
// PLAYBACK
//---------------------------------------------------------------------------
int alsa_playback(snd_pcm_t* handle, void* buffer, alsa_param_t* aparams)
{
	int i;
	int err = 0;
	
	// prepare
	if ((err = snd_pcm_prepare (handle)) < 0) 
	{
		fprintf (stderr, "Cannot prepare audio interface for use (%s)\n", snd_strerror (err));
		return err;
	}
	printf("Ready for playback...\n");

	for (i = 0; i < aparams->chunks; i++) 
	{
		err = snd_pcm_writei (handle, buffer, aparams->frames);
		if (err != aparams->frames) 
		{
	  		fprintf (stderr, "write to audio interface failed (%s)\n", snd_strerror (err));
	  		if (err == -EPIPE)
	  		{
	  			fprintf(stderr, "Underrun detected\n");
	  			if ((err = snd_pcm_prepare (handle)) < 0) 
				{
					fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));
					return err;
				}
	  		}
	  		if (err < 0)
				return err;
			fprintf(stderr, "Overrun detected\n");
		}
		printf("Playing...\r");
		fflush(stdout);
	}
	printf("\n");

	// drain playback stream, i.e., wait for all written samples to be played, and stop
	assert(snd_pcm_drain (handle) == 0); // something like fflush
	return 0;
} 
 
