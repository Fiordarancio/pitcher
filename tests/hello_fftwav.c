// FFTW: what if our input samples were tranformed, instead into the time domain?

// read the wav files and do the trasform
#include "fftw3.h"
#include "../wav.h"

#define FRAMES_PER_CHUNK 2205

int main(int argc, char** argv)
{
	// usage
	char* myfname = "pitches_32f/D.wav";
	char* mytname = "logs/trasform.txt";
			
	SNDFILE*		myfile = NULL;
	SF_INFO			myinfo;
	alsa_param_t	myparams;
	int 			i, c, err;
	FILE*			fdt = NULL;
	
	// open wav
	myfile = open_wav (myfname, &myinfo, &myparams);
	if (myfile == NULL)
	{
		fprintf(stderr, "Error while opening file %s\n", myfname);
		exit(EXIT_FAILURE);
	}
	// open file to append trasforms
	fdt = fopen(mytname, "a+");
	if (fdt == NULL)
	{
		fprintf(stderr, "Error while opening file %s\n", mytname);
		exit(EXIT_FAILURE);
	}
	
	// after the file is opened, we need to read the whole frames and prepare for fftw.
	// main difference between complex and real transforms is that the dimension
    // of in and out is not equal: in has n real values, while out has n/2 +1 values. 
    // This happens because, for Hermitian redundancy, out[i] = conjugate(out[i-1])
    double* 		in;
	fftw_complex*	out;
    fftw_plan 		p;
    
    in 	= fftw_malloc (sizeof(double) * FRAMES_PER_CHUNK);
	out = fftw_malloc (sizeof(fftw_complex) * FRAMES_PER_CHUNK);
    p 	= fftw_plan_dft_r2c_1d(FRAMES_PER_CHUNK, in, out, FFTW_ESTIMATE);
    
    int chunks_to_read = myparams.frames / FRAMES_PER_CHUNK;
    printf("We're going to transform %d chunks (surplus %d)\n", chunks_to_read, myparams.frames % FRAMES_PER_CHUNK);
    
    float* 	tmp_in = (float*) malloc (sizeof(float) * FRAMES_PER_CHUNK * myparams.channels);
    for (c=0; c<chunks_to_read; c++)
    {
		// initialize the input: read wav file
		err = read_wavchunk_float (myfile, tmp_in, FRAMES_PER_CHUNK, myparams.channels);
		if (err < 0)
		{
			fprintf(stderr, "Error while reading wavchunk. Closing...\n");
			fclose(fdt);
			close_wav(myfile);
			fftw_destroy_plan(p);
			fftw_free(in); fftw_free(out);
			exit(EXIT_FAILURE);
		}
		for (i=0; i<FRAMES_PER_CHUNK; i++)
			in[i] = (double) tmp_in[i];
		
		// once the plan is created, I can do the previous step about updating in as many 
		// times I need. Whenever I then apply the execute(plan), out is properly filled
		// with what we get
		fftw_execute(p); 
		
		// let's put the output in a file, just to see (use plot_fftw.py)
		for (i=0; i<(FRAMES_PER_CHUNK/2 +1); i++)
		{
			if ((err = fprintf(fdt, "%d,%f,%f\n", c*(FRAMES_PER_CHUNK/2 +1)+i, out[i][0], out[i][1])) < 0)
			{
				fprintf(stderr, "Error while appending: %s. Closing...\n", strerror(err));
				fclose(fdt);
				close_wav(myfile);
				fftw_destroy_plan(p);
				fftw_free(in); fftw_free(out);
				exit(EXIT_FAILURE);
			}
		}
	}

    
    // once we're done clear
    printf("Trasformation termined\n");
    free(tmp_in);
    fftw_destroy_plan(p);
    fftw_free(in); fftw_free(out);
	// close wav
	close_wav(myfile); 
	
	printf("\nHello World :D\n");
	return 0;
}

