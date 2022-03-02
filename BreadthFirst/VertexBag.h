#pragma once
#include "Pennant.h"

class VertexBag
{
    /* Bag data structure */
public:
    VertexBag();
    ~VertexBag();
    void PushVertex(int vertex);
    void PushPennant(Pennant* pen);
    VertexBag* Split();
    void Merge(VertexBag* other );
    bool Empty();
    int GetSize();        
    int largest;
    static const int bag_width = 32;
    Pennant* pennants[bag_width];
};

