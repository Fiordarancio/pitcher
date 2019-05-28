# check what the capturer thread produces
import numpy as np
import cmath
import matplotlib.pyplot as plt

f1 = 'logs/capturer_norm.txt'
f2 = 'logs/capturer_fft.txt'
	
t1 = open(f1, 'r')
t2 = open(f2, 'r')

time = [] # time step
samp = [] # sample
for line in t1: 
	elem = line.split(',')
	time.append(int(elem[0]))
	# already complex numbers' magnitude
	samp.append(float(elem[1].strip()))
print('Values:', len(time))

freq = [] # frequency
modl = [] # module
for line in t2: 
	elem = line.split(',')
	freq.append(int(elem[0]))
	modl.append(float(elem[1].strip()))
print('Values of fft:', len(modl))

frm = 2205
nin = 1103
# plot ONLY the first 3 sins
plt.plot(time[0:frm], samp[0:frm], 'r--', time[0:frm], samp[0:frm], 'r.', label='First signal')
plt.plot(time[frm:2*frm], samp[frm:2*frm], 'b--', time[frm:2*frm], samp[frm:2*frm], 'b.', label='Second signal')
plt.plot(time[2*frm:3*frm], samp[2*frm:3*frm], 'g--', time[2*frm:3*frm], samp[2*frm:3*frm], 'g.', label='Third signal')
plt.title('Raw audio signal (normalized)')
plt.xlabel('Time [s]')
plt.ylabel('Value [real number]')
plt.legend()

plt.figure()
plt.plot(freq[0:nin], modl[0:nin], 'r--', freq[0:nin], modl[0:nin], 'r.', label='First fft')
plt.plot(freq[nin:2*nin], modl[nin:2*nin], 'b--', freq[nin:2*nin], modl[nin:2*nin], 'b.', label='Second fft')
plt.plot(freq[2*nin:3*nin], modl[2*nin:3*nin], 'g--', freq[2*nin:3*nin], modl[2*nin:3*nin], 'g.', label='Third fft')
plt.title('Spectrum of signal')
plt.xlabel('Frequency [Hz]')
plt.ylabel('FFT magnitude')
plt.legend()

plt.show()

