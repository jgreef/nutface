
from itertools import izip

NUM_USERS = 480189
NUM_MOVIES = 17770

base = None
valid = None
hidden = None


def loadMatrix(setNum, matrix):
    for dataline, setline in izip(open("all.dta"), open("all.idx")):
        if(int(setline) == setNum): 
            print setline + " " + dataline


def getUserBaseMatrix():
    x = 1
def getUserValidMatrix():
    x = 1
def getUserHiddenMatrix():
    x = 1
def getUserProbeMatrix():
    x = 1
def getUserQualMatrix():
    x = 1



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


loadMatrix(2, 0)
