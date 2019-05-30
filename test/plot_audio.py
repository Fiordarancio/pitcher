# check what the capturer thread produces
import numpy as np
import cmath
import matplotlib.pyplot as plt

f1 = 'logs/hello_wav_audio.txt'
t1 = open(f1, 'r')

time = [] # time step
samp = [] # sample
for line in t1: 
	elem = line.split(',')
	time.append(int(elem[0]))
	# already complex numbers' magnitude
	samp.append(float(elem[1].strip()))
print('Values:', len(time))

plt.plot(time, samp, 'y--', time, samp, 'r.');
plt.xlabel('Time [s]')
plt.ylabel('Value [real number]')
plt.title('Audio wave')

plt.show()

