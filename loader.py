
from itertools import izip
from scipy.sparse import coo_matrix
from scipy.sparse import lil_matrix
import numpy as np

NUM_USERS = 480189
NUM_MOVIES = 17770

BASE_LEN = 
VALID_LEN =
HIDDEN_LEN =
PROBE_LEN = 
QUAL_LEN = 
base = None
valid = None
hidden = None
probe = None
qual = None


path = ""

#call this function if your data files are in a different directory
#from your python files
def initLoader(file_path):
    global path
    path = file_path 

#loads a the set corrosponding to setNum into matrix
def _loadMatrix(setNum, matrix):
    i = 0;
    for dataline, setline in izip(open(path+"all.dta"), open(path+"all.idx")):
        i += 1
        if i % 100000 == 0:
            print i
        if(int(setline) == setNum): 
            data = map(int, dataline.split())
            matrix[data[0], data[1]] = data[3]
    print "Done"
            
def _loadMatrix2(setnum):
    i = 0;

    for dataline, setline in izip(open(path+"all.dta"), open(path+"all.idx")):
        i += 1
        if i % 100000 == 0:
            print i
        if(int(setline) == setNum): 
            data = map(int, dataline.split())
            matrix[data[0], data[1]] = data[3]
    print "Done"

def getUserBaseMatrix():
    global base
    if base is None:
        matrix = lil_matrix((NUM_USERS, NUM_MOVIES), dtype = np.int8)
        _loadMatrix(1, matrix)
        base = matrix
    return base
def getUserValidMatrix():
    global valid
    if valid is None:
        matrix = lil_matrix((NUM_USERS, NUM_MOVIES), dtype = np.int8)
        _loadMatrix(1, matrix)
        valid = matrix
    return valid
def getUserHiddenMatrix():
    global hidden 
    if hidden is None:
        matrix = lil_matrix((NUM_USERS, NUM_MOVIES), dtype = np.int8)
        _loadMatrix(1, matrix)
        hidden = matrix
    return hidden 
def getUserProbeMatrix():
    global probe
    if qual is None:
        matrix = lil_matrix((NUM_USERS, NUM_MOVIES), dtype = np.int8)
        _loadMatrix(1, matrix)
        probe = matrix
    return probe
def getUserQualMatrix():
    global qual 
    if qual is None:
        matrix = lil_matrix((NUM_USERS, NUM_MOVIES), dtype = np.int8)
        _loadMatrix(1, matrix)
        qual = matrix
    return qual



def getMovieBaseMatrix():
    x = 1
def getMovieValidMatrix():
    x = 1
def getMovieHiddenMatrix():
    x = 1
def getMovieProbeMatrix():
    x = 1
def getMovieQualMatrix():
    x = 1

#initLoader("mu/")

#getUserBaseMatrix()

f = open("mu/all.idx")

count1 = 0
count2 = 0
count3 = 0
count4 = 0
count5 = 0
i = 0
for line in f:
    i += 1
    if i % 100000 == 0:
        print i
    if int(line) == 1:
        count1 += 1
    if int(line) == 2:
        count2 += 1
    if int(line) == 3:
        count3 += 1
    if int(line) == 4:
        count4 += 1
    if int(line) == 5:
        count5 += 1
print "count1 " + str(count1)
print "count2 " + str(count2)
print "count3 " + str(count3)
print "count4 " + str(count4)
print "count5 " + str(count5)

