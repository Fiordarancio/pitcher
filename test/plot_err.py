# read a stream of DFT arrays and plot them
import numpy as np
#import matplotlib
#matplotlib.use('agg') # needed against tkinter
import matplotlib.pyplot as plt


# Default values are often updated
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

times = 0
for i in epoch:
	if i==1:
		times += 1
print('There are', times, 'plots to do')

# plotting with colors
if times == 2:
	plt.plot(batch[1:len(batch)//2], lerr[1:len(lerr)//2], 'r--', label='First plot')
	plt.plot(batch[1:len(batch)//2], lerr[1:len(lerr)//2], 'r.')
	plt.plot(batch[len(batch)//2:len(batch)], lerr[len(lerr)//2:len(lerr)], 'b--', label='Second plot')
	plt.plot(batch[len(batch)//2:len(batch)], lerr[len(lerr)//2:len(lerr)], 'b.')
	plt.title('Local error over batches (all epochs)') 
	plt.xlabel('Batch')
	plt.ylabel('Error')
	plt.legend()

	plt.figure()
	plt.plot(epoch[1:len(epoch)//2], gerr[1:len(gerr)//2], 'r--', label='First plot')
	plt.plot(epoch[1:len(epoch)//2], gerr[1:len(gerr)//2], 'r.')
	plt.plot(epoch[len(epoch)//2:len(epoch)], gerr[len(gerr)//2:len(gerr)], 'b--', label='Second plot')	
	plt.plot(epoch[len(epoch)//2:len(epoch)], gerr[len(gerr)//2:len(gerr)], 'b.')
	plt.title('Global error over epochs') 
	plt.xlabel('Epoch')
	plt.ylabel('Error')
	plt.legend()
elif times == 3:
	plt.plot(batch[1:len(batch)//3], lerr[1:len(lerr)//3], 'r--', label='First plot')
	plt.plot(batch[1:len(batch)//3], lerr[1:len(lerr)//3], 'r.')
	plt.plot(batch[len(batch)//3:2*len(batch)//3], lerr[len(lerr)//3:2*len(lerr)//3], 'b--', label='Second plot')
	plt.plot(batch[len(batch)//3:2*len(batch)//3], lerr[len(lerr)//3:2*len(lerr)//3], 'b.')
	plt.plot(batch[2*len(batch)//3:len(batch)], lerr[2*len(lerr)//3:len(lerr)], 'g--', label='Third plot')
	plt.plot(batch[2*len(batch)//3:len(batch)], lerr[2*len(lerr)//3:len(lerr)], 'g.')
	plt.title('Local error over batches (all epochs)') 
	plt.xlabel('Batch')
	plt.ylabel('Error')
	plt.legend()

	plt.figure()
	plt.plot(epoch[1:len(epoch)//3], gerr[1:len(gerr)//3], 'r--', label='First plot')
	plt.plot(epoch[1:len(epoch)//3], gerr[1:len(gerr)//3], 'r.')
	plt.plot(epoch[len(epoch)//3:2*len(epoch)//3], gerr[len(gerr)//3:2*len(epoch)//3], 'b--', label='Second plot')
	plt.plot(epoch[len(epoch)//3:2*len(epoch)//3], gerr[len(gerr)//3:2*len(epoch)//3], 'b.')
	plt.plot(epoch[2*len(epoch)//3:len(epoch)], gerr[2*len(gerr)//3:len(gerr)], 'g--', label='Third plot')
	plt.plot(epoch[2*len(epoch)//3:len(epoch)], gerr[2*len(gerr)//3:len(gerr)], 'g.')
	plt.title('Global error over epochs') 
	plt.xlabel('Epoch')
	plt.ylabel('Error')
	plt.legend()
else:
	plt.plot(batch, lerr, 'r--', )
	plt.plot(batch, lerr, 'r.')
	plt.title('Local error over batches (all epochs)') 
	plt.xlabel('Batch')
	plt.ylabel('Error')

	plt.figure()
	plt.plot(epoch, gerr, 'r--')
	plt.plot(epoch, gerr, 'r.')
	plt.title('Global error over epochs') 
	plt.xlabel('Epoch')
	plt.ylabel('Error')

plt.show()


