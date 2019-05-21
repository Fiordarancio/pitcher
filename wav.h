//-----------------------------------------------------------------------------------------------------
// GENERALIZED WAV READER (library)
// Read and playback the WAV file. lsndfile allows variations between data to be read
//-----------------------------------------------------------------------------------------------------
#ifndef WAV_H
#define WAV_H

#include <stdio.h>
#include <stdlib.h>

#include "autil.h"
#include <sndfile.h>

//#define VERBOSE
#define ERR_CHUNK_INCOMPLETE (11)

//-----------------------------------------------------------------------------------------------------
// OPEN & CLOSE A WAV FILE
//-----------------------------------------------------------------------------------------------------
SNDFILE* open_wav (char* fname, SF_INFO* info, alsa_param_t* params);
int close_wav(SNDFILE* file);
//-----------------------------------------------------------------------------------------------------
// READ WAV CHUNKS (of different types)
// This function reads a chunk of N frames and stores it. It is possible to start
// the reading at a specified offset. 
//-----------------------------------------------------------------------------------------------------
int read_wavchunk_short (SNDFILE* wav_file, short* buffer, int frames, int channels);
int read_wavchunk_int (SNDFILE* wav_file, int* buffer, int frames, int channels);
int read_wavchunk_float (SNDFILE* wav_file, float* buffer, int frames, int channels);
//-----------------------------------------------------------------------------------------------------
// READ AND PLAY a wav file entirely
//-----------------------------------------------------------------------------------------------------
int readnplay_wav_short	(SNDFILE* wav_file, SF_INFO* wav_info, snd_pcm_t* handle, alsa_param_t* params);
int readnplay_wav_int 	(SNDFILE* wav_file, SF_INFO* wav_info, snd_pcm_t* handle, alsa_param_t* params);
int readnplay_wav_float	(SNDFILE* wav_file, SF_INFO* wav_info, snd_pcm_t* handle, alsa_param_t* params);
int readnplay_wav_double(SNDFILE* wav_file, SF_INFO* wav_info, snd_pcm_t* handle, alsa_param_t* params);

#endif

