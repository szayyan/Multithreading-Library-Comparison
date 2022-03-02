#pragma once

struct ParallelData
{
    /* Shared data for all parallel classes */ 
    int n;
    int num_threads;
    int thread_or_omp;
};