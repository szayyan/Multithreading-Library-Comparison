import sys
import subprocess

if len(sys.argv) != 2:
    print( "Must be an args <input_executable>")
    quit()

def pow_of_2(a):
    return (a & (a-1)) == 0 and a != 1

processor_counts = []
thread_counter = 0
increment = 1

# we dont want to measure every single thread count from 1-64 - extremely time consuming
# we measure more closer to 0
while thread_counter < 64:
    if pow_of_2(thread_counter) and thread_counter >= 8:
        increment *= 2
    thread_counter += increment
    processor_counts.append(str(thread_counter))

algos = ["boids","chull","bfs"]

for i in range(1):
    with open(algos[i]+".txt","w") as out:
        for threads in processor_counts:
            for lib in ["1","2"]:
                if (lib == "2" and not pow_of_2(int(threads))):
                    continue
                output = subprocess.check_output([sys.argv[1],"-algo",str(i+1), "-threads",threads,"-lib",lib])
                out.write(str(output))
                out.write(",")
            out.write("\n")