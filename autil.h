//-------------------------------------------------------------------------------------
// ALSA UTILITES
// Functions for:
//   - setting harware and software paramenters in ALSA
//   - executing playback or capture
//-------------------------------------------------------------------------------------
#ifndef ALSAUTIL_H
#define ALSAUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

// defaults values
#define DEF_CHANNELS	1
#define DEF_SAMPLERATE	44100
#define DEF_FRAMES		4096
#define DEF_CHUNKS		1
#define DEF_FORMAT		(SND_PCM_FORMAT_S16_LE)	
#define DEF_DEV			("default")
#define MODE_PLAY		0
#define MODE_CAPT		1

//----------------------------------------------------------------------------------------------
// VARIABLES
//----------------------------------------------------------------------------------------------
typedef struct {
	unsigned int samplerate;	// sampling rate
	unsigned int channels;		// number of channels
	unsigned int frames;		// frames 
	unsigned int chunks;		// each chunck has #samples = frames*channels
	unsigned int format;	 	// example: S16_LE, S32_BE...
} alsa_param_t;

snd_pcm_t* 	capture_handle;
snd_pcm_t* 	playback_handle;
char* 		capture_device; 
char* 		playback_device;

//----------------------------------------------------------------------------------------------
// OPEN & CLOSE HANDLE
//----------------------------------------------------------------------------------------------
snd_pcm_t* 	alsa_open(char* device, int mode);
int 		alsa_close(snd_pcm_t* handle);
//----------------------------------------------------------------------------------------------
// BASIC CONFIGURATION OF HIGH-LEVEL PARAMETERS
//----------------------------------------------------------------------------------------------
void 		alsa_param_init(alsa_param_t* aparams);
void 		alsa_param_print(alsa_param_t* aparams);
//----------------------------------------------------------------------------------------------
// HARDWARE PARAMETERS CONFIGURATION 
//----------------------------------------------------------------------------------------------
int 		alsa_hw_param_config (snd_pcm_t* handle, alsa_param_t* aparams);
//----------------------------------------------------------------------------------------------
// XRUN RECOVERY (UNDERRUN/OVERRUN)
// In both cases, the interface was not fast enough to deliver information from/to the device. 
// An overrun occurs when the audio device cannot provide data to process quickly enough.
/// Usually, it happens when you run at very low buffer size while the sound card should process 
// incoming buffers very fast.
// An underrun occurs when the application can't pass data to the audio buffer quickly enough.
//----------------------------------------------------------------------------------------------
int 		xrun_recovery(snd_pcm_t *handle, int err, unsigned long usec);
//----------------------------------------------------------------------------------------------
// CAPTURE & PLAYBACK
// Different kind of functions can be chosen according to the format in use
//----------------------------------------------------------------------------------------------
int 		alsa_capture_float(snd_pcm_t* handle, float* buffer, alsa_param_t* aparams);
int			alsa_capture_int(snd_pcm_t* handle, int* buffer, alsa_param_t* aparams);
int 		alsa_playback(snd_pcm_t* handle, void* buffer, alsa_param_t* aparams);
 
#endif

