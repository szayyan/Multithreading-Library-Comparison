# OpenMP vs C++ threading

This program is designed to compare the differences in the libraries OpenMP and C++ standard library threading through the implementation of three algorithms. The program has two key parts - visualisation of thread usage in the form of a graphical application and simple benchmarking tool for performance comparison. A short demonstration of the graphical application is available to view here: https://www.youtube.com/watch?v=dQu_TXzs1VM
## Requirements
The following must be installed to build successfully:
```bash
freeglut3
freeglut3-dev
libglew1.5
libglew1.5-dev
libglu1-mesa
libglu1-mesa-dev
libgl1-mesa-glx
libgl1-mesa-dev 
libsfml-dev
```
Minimum required versions
```bash
OpenGL >= 3.3
OpenMP >= 4.5
g++ >= 6.1 if using gcc
```

## Usage
| Parameter | Options | Default | Description |
| --- | --- | --- | --- |
|-algo   |   1-3   |    1    |      changes what algorithm is run. 1 refers to the flocking simulation, 2 is convex hull and 3 is breadth first search.
-threads | >=1 | hardware default | how many threads are running the algorithm
-lib | 1-2 | 1 | what library implementation to use. 1 for OpenMP and 2 for C++ threading.
-benchmark | 0-1 | 0 | 0 launches the visualisation 1 launches the benchmark
-out | 0-1 | 1 | 1 prints useful data to the console 0 is used mainly for profiling with external tools.
-n | >1 | varies on algorithm | For flocking refers to how many boids there are in the scene, convex hull refers to the number of points in the point cloud and bfs refers to the number of vertices in the graph.

All parameters are optional and will default to appropiate values if not chosen
 
Usage example:
```bash
./MPvsX -algo 1 -threads 3 -lib 2 -out 0 -benchmark 0 
```
## License
[MIT](https://choosealicense.com/licenses/mit/)

