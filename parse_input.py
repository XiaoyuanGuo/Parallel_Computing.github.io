import numpy as np
import sys
# this is for quadrilateral, data preprocessing
def main(filename):
    n = 0
    with open(filename) as f:
        line = f.readline()
    	cnt = 1
    	while line:
            if line[0] == '#':
    		line = f.readline()
    		continue
            a, b = line.split()
	    
            if int(a) > int(b):
                t = a
            	a = b
            	b = t
	    if int(a) == int(b):
	        line = f.readline()
	        continue
            with open(filename[:-4]+'-new.txt', 'a') as fp:
     		fp.write(a+' '+b+'\n')
            	fp.write(b+' '+a+'\n')
            n += 1
            line = f.readline()
    print n

if __name__ == "__main__":
   main(sys.argv[1])       	
