import math


def get_thread_bin( pos, no_threads, dimension ):
    n = int(math.log2(no_threads))
    s = [1,1,1]
    div_ctr = 0
    for i in range(n,0,-1):
        s[div_ctr] *= 2
        div_ctr = (div_ctr + 1) % 3
    print(s)
    npos = [ min(int((pos[i]*s[i]) // dimension),s[i]-1) for i in range(len(pos))]
    print( "{} is translated to {}".format(pos,npos))

    #return sum( for i in npos)

dim = 5
for x in range(dim):
    for y in range(dim):
        for z in range(dim):
           get_thread_bin([x,y,z],16,dim)

