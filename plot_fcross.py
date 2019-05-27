# read the check files for fcross
import numpy as np
import cmath
import matplotlib.pyplot as plt

tfile1 = 'logs/fcross_fftw.txt'
tfile3 = 'logs/fcross_norm.txt'
	
t1 = open(tfile1, 'r')
t3 = open(tfile3, 'r')

samp1 = []
freq1 = []
for line in t1: 
	elem = line.split(',')
	samp1.append(int(elem[0]))
	# already complex numbers' magnitude
	freq1.append(float(elem[1].strip()))
print('Values:', len(samp1))

print('Max frequency values:', np.abs(np.max(freq1)))

time = []
vals = []
for line in t3: 
	elem = line.split(',')
	time.append(int(elem[0]))
	vals.append(float(elem[1].strip()))
print('Values of sinew:', len(vals))
print('Time instants:', len(time))

# plot now the real frequency, in magnitude 
Fs = 44100		# samples per second 
T  = 1/Fs 		# period per sample
L1 = len(samp1)	# lenght of signal (already considering only N/2 +1 values)
x1 = np.arange(0, Fs//2, Fs/(2*L1))
print('number of frequency steps:', len(x1))
plt.plot(x1, np.abs(freq1), 'y--', x1, np.abs(freq1), 'r.')
plt.title('Spectrum of signal')
plt.xlabel('Frequency [Hz]')
plt.ylabel('FFT magnitude |DFT(f)|')

#x2 = np.arange(0, Fs//2, Fs/(2*L2))
#print('number of frequency steps:', len(x2))
#plt.figure()
#plt.plot(x2, np.abs(freq2), 'y--', x2, np.abs(freq2), 'r.')
#plt.title('Spectrum of channel 2')
#plt.xlabel('Frequency [Hz]')
#plt.ylabel('FFT magnitude |DFT(f)|')

plt.figure()
plt.plot(time, vals, 'y--', time, vals, 'r.')
plt.title('Normalized sine wave')
plt.xlabel('time [ms]')
plt.ylabel('sample')

plt.show()


