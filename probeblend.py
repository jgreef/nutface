
from numpy import matrix
from numpy import linalg
import time
import numpy

import sys

files = []
filenames = []




for arg in sys.argv[1:]:
    print arg
    files.append(open(arg))
    filenames.append(arg)


filenames = [s.replace("probe", "qual") for s in filenames]

qualfiles = []
for f in filenames:
    qualfiles.append(open(f))
    print f

X = numpy.zeros(shape=(1374739, len(files)))
y = numpy.zeros(shape=(1374739, 1))

i = 0
for line in open("probe.dta"):
    y[i] = [numpy.float32(line)]
    i += 1
y = matrix(y)


print "adf"
i = 0
while i < 1374739:
    dataline = [];
    for f in files:
        x = f.readline()
        dataline.append(numpy.float32(x))
        
    X[i] = dataline;
    i += 1


print "X: ", X
print "y: ", y
X = matrix(X)
w = ((X.T*X).I * X.T) * y
print "w: ", w


X = numpy.zeros(shape=(2749898, len(qualfiles)))

i = 0
while i < 2749898:
    dataline = [];
    for f in qualfiles:
        x = f.readline()
        dataline.append(numpy.float32(x))
    X[i] = dataline;
    i += 1

X = matrix(X)

s = X * w

print "s: ", s


outname = time.strftime("blend_output_%d_%m_%Hh%Mm.out") 

outfile = open(outname,"w")
for i in range(2749898):
    outfile.write(str(s[i][0])[2:-2] + "\n")
    if i % 10000 == 0:
        print i
