import re
import matplotlib.pyplot as plt
from scipy import interpolate
import numpy as np
from sys import argv

'''use "all" keyword as parameter to print all the cores, put the name/path of the file as parameter to use
   that file instead of "trace" (put the name of the file as last parameter) '''

def plot_temp_all():
	x=np.arange(0, len, 1)
	colour=('k', 'r', 'g', 'b')
	for i in [0,2,4,6]:
		y=np.array(dati_temp[str(i)])
		f=interpolate.interp1d(x, y)
		ynew=f(x)
		plt.plot(x, ynew, colour[int(i/2)], linewidth=0.5, label='Core'+str(int(i/2)))

def plot_freq_all():
	x=np.arange(0, len, 1)
	colour=('k', 'r', 'g', 'b')
	for i in [0,2,4,6]:
		y=np.array(dati_freq[str(i)])
		f=interpolate.interp1d(x, y)
		ynew=f(x)
		plt.plot(x, ynew, colour[int(i/2)], linewidth=0.5, label='Core'+str(int(i/2)))

def plot_temp():
	x=np.arange(0, len, 1)
	y=np.array(dati_temp['0'])
	f=interpolate.interp1d(x, y)
	ynew=f(x)
	plt.plot(x, ynew, 'k', linewidth=0.5, label='Core0')

def plot_freq():
	x=np.arange(0, len, 1)
	y=np.array(dati_freq['0'])
	f=interpolate.interp1d(x, y)
	ynew=f(x)
	plt.plot(x, ynew, 'k', linewidth=0.5, label='Core0')

file=None
try:
	if(argv[-1]=='plot.py'):
		raise Exception
	file=open(argv[-1], 'r')
	print("Stampo "+argv[-1]+"\n")
except Exception:
	file=open('trace','r');
	print("Stampo trace\n")
log=file.readlines()
file.close()
camp=int(input("Inserisci il campionamento in ms (min 5ms): "))
camp=int(camp/5)
core=0
freq=0
temp=0
dati_temp={ '0' : [], '1' : [], '2' : [], '3' : [], '4' : [], '5' : [], '6' : [], '7' : []}
dati_freq={ '0' : [], '1' : [], '2' : [], '3' : [], '4' : [], '5' : [], '6' : [], '7' : []}
delay=0
for words in log:
	if(delay==0):
		if('Core' in words):
			core=words[-2]
		if('Target Frequency' in words):
			freq=words[-3:-1]
			dati_freq[str(core)]=dati_freq[str(core)]+[freq]
		if('Temperature' in words):
			temp=words[-5:-3]
			dati_temp[str(core)]=dati_temp[str(core)]+[temp]
	if('-----------' in words):
		delay=delay+1
		if(delay==camp):
			delay=0

len=len(dati_temp['0'])
print('Numero di campioni: '+str(len))
print('Max temp core 0: '+str(max(dati_temp['0'])))
print('Max temp core 1: '+str(max(dati_temp['2'])))
print('Max temp core 2: '+str(max(dati_temp['4'])))
print('Max temp core 3: '+str(max(dati_temp['6'])))


plt.subplot(2, 1, 1)
if('all' in argv):
	plot_temp_all()
else:
	plot_temp()
plt.legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
plt.xlabel('Time ['+str(camp*5)+'ms]')
plt.ylabel('Temperature [°C]')
plt.subplot(2, 1, 2)
if('all' in argv):
	plot_freq_all()
else:
	plot_freq()
plt.xlabel('Time ['+str(camp*5)+'ms]')
plt.ylabel('Frequency Multiplier')
plt.show()