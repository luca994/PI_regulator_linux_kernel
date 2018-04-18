import re
import matplotlib.pyplot as plt
from scipy import interpolate
import numpy as np

def plot_temp():
	x=np.arange(0, len, 1)
	y=np.array(dati_temp['0'])
	f=interpolate.interp1d(x, y)
	ynew=f(x)
	plt.plot(x, ynew, 'k', linewidth=0.5)

def plot_freq():
	x=np.arange(0, len, 1)
	y=np.array(dati_freq['0'])
	f=interpolate.interp1d(x, y)
	ynew=f(x)
	plt.plot(x, ynew, 'k', linewidth=0.5)

file=open("trace",'r');
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
		if('Actual Frequency' in words):
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

plt.subplot(2, 1, 1)
plot_temp()
plt.subplot(2, 1, 2)
plot_freq()
plt.show()