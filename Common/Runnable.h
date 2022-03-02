#pragma once

class Runnable
{
    /* Abstract parent class for all benchmarkable algorithms */
public:
    virtual void Run() = 0;
    virtual void OutputStats() = 0;
    virtual ~Runnable() {};
};