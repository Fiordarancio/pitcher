# read a stream of DFT arrays and plot them
import numpy as np
#import matplotlib
#matplotlib.use('agg') # needed against tkinter
import matplotlib.pyplot as plt


#gfile = 'logs/ftrain_glberr.txt'
#lfile = 'logs/ftrain_locerr.txt'
gfile = 'logs/hello_testchar_glberr.txt'
lfile = 'logs/hello_testchar_locerr.txt'
ans = input('Can I use files in log (now {} and {})? [y/n] '.format(gfile, lfile))
if (ans != 'y'):
	gfile = input('Please tell me the file to read for GLOBAL error: ')
	lfile = input('Please tell me the file to read for LOCAL error: ')

g = open(gfile, 'r')
l = open(lfile, 'r')
#f = open('../retis.sssup.it/fft/dft.txt', 'r')
#print(f.read()) 

batch = []
lerr  = []
epoch = []
gerr  = []
for line in l: # local
	elem = line.split(',')
	batch.append(int(elem[0]))
	lerr.append(float(elem[1].strip()))
print('Values:', len(batch), ',', len(lerr))

for line in g: # global
	elem = line.split(',')
	epoch.append(int(elem[0]))
	gerr.append(float(elem[1].strip()))
print('Values:', len(epoch), ',', len(gerr))

# plotting
plt.plot(batch, lerr, 'r--')
plt.plot(batch, lerr, 'b.')
plt.title('Local error over batches (all epochs)') 
plt.xlabel('Batch')
plt.ylabel('Error')

plt.figure()
plt.plot(epoch, gerr, 'r--')
plt.plot(epoch, gerr, 'b.')
plt.title('Global error over epochs') 
plt.xlabel('Epoch')
plt.ylabel('Error')

plt.show()


