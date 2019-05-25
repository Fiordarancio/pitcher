# PITCHER - A neural-PITCH-recognizER
C implementation of a pitch recognizer using a perceptron-based neural network.

## Brief description
The main service the application is goig to deliver concerns:
* building and training the newtork using different audio samples (synthetized and/or wav)
* capture audio from an input device and ubmit to neural network
* output results on terminal

The project is deployed under a Linux operating system and exploits [ALSA](https://alsa-project.org/alsa-doc/alsa-lib/index.html) for audio devices control. For wav reading, we used [libsndfile](http://www.mega-nerd.com/libsndfile/). Neural network is self-implemented.

The audio sampling rate we refer by now is 44100 Hz and this is relevant since determines the number of frames effectively read in the period the capturing thread is called (50 ms). Though, both these values can be changed where they are defined, before compilation.

## Map
The main folder contains (.h and .c):

### Libraries
* `autil`		: utility mini-library for easily manage the ALSA interfaces
* `wav`			: library for reading a wav file by chunks or entirely, testing also playback if needed
* `pnet`		: library with all functions needed to build a multi-layer perceptron network, to train it and save its weights
* `mutils`		: additional geormetric functions
* `ptask_time`	: library for time thread utilities
* `pnetlib`		: contains macro-function about: the building of a training set of elements called `example`s, formed by an array of real numbers for samples and another for the label (this generalization allows the user to create any kind of input, as long as he builds up the network accordingly); the training algorithm (we use Stochastic Gradient Descent on minibatch); error evaluation and printing; set shuffling and normalization; plus other utility functions needed during training

### Main programs
There are 2 versions of the main programs: `train/ftrain` and `pitcher/fpitcher`. The difference is the application or not of the fft in the examples given (and predicted). In both trainings, we build a 3 layer perceptron network with 12 output neurons (12 pitches to recognize) and save obtained weights. 
Variables to be set at run time: 
* the number of input and hidden neurons
* common bias (trainable)
* max number of epochs
* batch size
* learning rate
* momentum

See `DATA.md` for some experimental hints about those values (NOT UP TO DATE).

In both pitch-recognizers, the network saved after a training is used by a thread to recognize chunks of data read by an ALSA capturer thread.

### Plotting utilities
The file `plot_err.py` is a minimal python3 program that plots error (global - epoch and local - batch) saved after a training usually in `logs/<executable_name>_glberfile.txt` and `logs/<executable_name>locerfile.txt`. 
The file `plot_fft.py` is connected specifically to `hello_fft` and it is used to plot spectrum of a test signal. Similarly, `plot_ftrain.py` does the same thing but it is designed for a check of the prepared training set used by `ftrain`.

### Test programs
The `tests` folders contains a bunch of tests that, using special network and input configuration, are meant to check performances and find errors. All programs with `hello_` as prefix indicate test programs: descriptions can be found in the files. The program `pitcher3` is a test too, but kept in the main directory: generally it is useful for tuning microphone boosting of your pc (try `alsamixer`). 

### Data folders
The folder `pitches_32f` containes .wav files that record some pitches of digital piano. They are used to extract samples for training. 
Since, by now, we do not implement an automatic procedure for reading all files in a folder and categorize them by name (to know the pitch), if you want to add more files to enlarge your training set, first make sure that they have single-channel 32-bit format sampled at 44.100 KHz. When the files are loaded, add their names in order of pitch into `train.c`.

The folder `logs` contains relevant files about the last training that has been launched (`<executable_name>.txt`) and saved weights and error values, which can be plot using python utilities. Use `plot_err.py` with `hello_<test_name>_[glb/loc]err.txt` to plot error during training under certain conditions. 

## Compile
If you want to create a new network, train it and use it into pitcher, do:
* `$ make clean`
* `$ make all` (prepares `train`/`pitcher` or `ftrain`/`fpitcher`)
Use `$ make ct` every time you want to lauch a new train, otherwise plots about error monitoring will not be overwritten. Then, launch `./train` and follow the steps to get the work done. All relevant files will be saved into the `logs/` folder.

While compiling all programs, including tests, do:
* `$ make universe`

**NOTE**: the majority of log files, especially *log error* ones, are written in the append mode. Remember to remove/rename the last ones before launching your programs, otherwise you'll corrupt your results.

# A closer look to `ftrain`
The program `ftrain` trains a perceptron based network whose tunable parameters are the following:
* number of neurons in hidden layer
* initial common bias for all neurons
* objective function
* batch size
* max number of epoch to reach while training
* learning rate and momentum
* min error to reach for early stopping

In this file we are going to list the values that leaded to best results, alongside with the failure rate gained over the training set itself used as a test set for now. In a further development, we are going to use also a test set.

## Input data
The training set is created synthetizing sinusoidal signals with known frequency and volume: a stronger training set should include samples with some noise from the base frequencies too. In order to create the test set, we should try to mix some good examples and some noisy ones. 


