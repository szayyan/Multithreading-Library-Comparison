# before i realised make clean existed :)
from os import listdir,remove

for i in listdir():
    if i.split(".")[-1] == "o":
       remove(i)

