# read a stream of DFT arrays and plot them
import numpy as np
import cmath
#import matplotlib
#matplotlib.use('agg') # needed against tkinter
import matplotlib.pyplot as plt

tfile1 = 'logs/hello_fft_sinc1.txt'
tfile2 = 'logs/hello_fft_sinc2.txt'
ans = input('Can I use tfile1 in log ({} and {})? [y/n] '.format(tfile1, tfile2))
if (ans != 'y' and ans != 'yes'):
	tfile1 	= input('Please tell me the tfile1 to read for Fourier Trasform channel 1: ')
	tfile2 	= input('Please tell me the tfile2 to read for Fourier Trasform channel 2: ')
# else they're set	
	
t1 = open(tfile1, 'r')

samp1 = []
freq1 = []
for line in t1: 
	elem = line.split(',')
	samp1.append(int(elem[0]))
	re = float(elem[1].strip())
	im = float(elem[2].strip())
	freq1.append(complex(re, im))
print('Values:', len(samp1))

t2 = open(tfile2, 'r')

samp2 = []
freq2 = []
for line in t2: 
	elem = line.split(',')
	samp2.append(int(elem[0]))
	re = float(elem[1].strip())
	im = float(elem[2].strip())
	freq2.append(complex(re, im))
print('Values:', len(samp2))

print('Max frequency values:', np.abs(np.max(freq1)),' and', np.abs(np.max(freq2)))

t3 = open('logs/hello_fft_sinew.txt', 'r')
time = []
vals = []
for line in t3: 
	elem = line.split(',')
	time.append(int(elem[0]))
	vals.append(float(elem[1].strip()))
print('Values of sinew:', len(vals))
print('Time instants:', len(time))


# consider that for the trasform of real values, the first N//2 elements are
# conjugate of the second N//2. Moreover, while plotting, we need to take the 
# MAGNITUDE of the complex number: that is the frequency.

# plotting positive frequencies
#plt.plot(samp1[0:len(samp1)//2], np.abs(freq1[0:len(samp1)//2]), 'xkcd:silver')
#plt.plot(samp1[0:len(samp1)//12], np.abs(freq1[0:len(samp1)//12]), 'm.')
#plt.plot(samp1[len(samp1)//12:len(samp1)//6], np.abs(freq1[len(samp1)//12:len(samp1)//6]), 'b.')
#plt.plot(samp1[len(samp1)//6:len(samp1)//4], np.abs(freq1[len(samp1)//6:len(samp1)//4]), 'g.')
#plt.plot(samp1[len(samp1)//4:5*len(samp1)//12], np.abs(freq1[len(samp1)//4:5*len(samp1)//12]), 'y.')
#plt.plot(samp1[5*len(samp1)//12:len(samp1)//2], np.abs(freq1[5*len(samp1)//12:len(samp1)//2]), 'r.')
#plt.title('Positive Fourier Trasform of wav file') 
#plt.xlabel('Sample')
#plt.ylabel('Frequency')

# plot negative frequency (just to see) 
#plt.plot(samp1[len(samp1)//2:len(samp1)], np.abs(freq1[len(samp1)//2:len(samp1)]), 'xkcd:silver')
#plt.title('Negative Fourier Trasform of wav file') 
#plt.xlabel('Sample')
#plt.ylabel('Frequency')

# plot now the real frequency, in magnitude 
Fs = 44100		# samples per second 
T  = 1/Fs 		# period per sample
L1 = len(samp1)	# lenght of signal
L2 = len(samp2)
x1 = np.arange(0, Fs//2, Fs/(2*L1))
print('number of frequency steps:', len(x1))
plt.plot(x1, np.abs(freq1), 'y--', x1, np.abs(freq1), 'r.')
plt.title('Spectrum of channel 1')
plt.xlabel('Frequency [Hz]')
plt.ylabel('FFT magnitude |DFT(f)|')

x2 = np.arange(0, Fs//2, Fs/(2*L2))
print('number of frequency steps:', len(x2))
plt.figure()
plt.plot(x2, np.abs(freq2), 'y--', x2, np.abs(freq2), 'r.')
plt.title('Spectrum of channel 2')
plt.xlabel('Frequency [Hz]')
plt.ylabel('FFT magnitude |DFT(f)|')

plt.figure()
plt.plot(time, vals, 'y--', time, vals, 'r.')
plt.title('Original sine wave')
plt.xlabel('time [ms]')
plt.ylabel('sample')

plt.show()


