#include "VertexBag.h"
#include <iostream>
#include <math.h>

VertexBag::VertexBag():
largest(-1),
pennants{nullptr}
{
}

bool VertexBag::Empty()
{ 
    return largest < 0; 
}

int VertexBag::GetSize() 
{
    int size = 0;
    for(size_t i = 0; i < largest + 1; i++)
    {
        if( pennants[i] != nullptr )
            size += std::pow(2,i);
    }
    return size;
}

void VertexBag::PushVertex(int vertex)
{
    /* Adds a new vertex to the bag */
    Pennant* p = new Pennant(new Node(vertex));
    PushPennant(p);
}

void VertexBag::PushPennant(Pennant* pen)
{
    /* Adds a new pennant to the bag*/
    int index = 0;
    while( pennants[index] != nullptr )
    {
        pen = pennants[index]->Union(pen);
        pennants[index++] = nullptr;
    }
    pennants[index] = pen;
    largest = std::max(largest,index);
}

void VertexBag::Merge(VertexBag* other)
{
    /* Merges two bags together - does not handle memory deletion for other bag */

    int largest_both = std::max(largest,other->largest);
    Pennant* leftover = nullptr;
    for(int i = 0; i < largest_both + 1; i++)
    {
        int code =  (pennants[i] != nullptr) | // 1
                    ((other->pennants[i] != nullptr) << 1) | // 2
                    ((leftover != nullptr) << 2);        // 4

        switch (code)
        {
            case 0: // pennants[i] = nullptr, other->pennants[i] = nullptr, leftover = nullptr
                break;
            case 1: // pennants[i] != nullptr, other->pennants[i] = nullptr, leftover = nullptr
                break;
            case 2: // pennants[i] = nullptr, other->pennants[i] != nullptr, leftover = nullptr
                pennants[i] = other->pennants[i];
                other->pennants[i] = nullptr;
                break;
            case 3: // pennants[i] != nullptr, other->pennants[i] != nullptr, leftover = nullptr
                leftover = pennants[i]->Union(other->pennants[i]);
                pennants[i] = nullptr;
                break;
            case 4: // pennants[i] = nullptr, other->pennants[i] = nullptr, leftover != nullptr
                pennants[i] = leftover;
                leftover = nullptr;
                break;
            case 5: // pennants[i] ""= nullptr, other->pennants[i] = nullptr, leftover != nullptr
                leftover = pennants[i]->Union(leftover);
                pennants[i] = nullptr;
                break;
            case 6: // pennants[i] = nullptr, other->pennants[i] != nullptr, leftover != nullptr
                leftover = other->pennants[i]->Union(leftover);
                other->pennants[i] = nullptr;
                break;
            case 7: // pennants[i] != nullptr, other->pennants[i] != nullptr, leftover != nullptr
                leftover = other->pennants[i]->Union(leftover);
                other->pennants[i] = nullptr;
                break;
        }
    }


    if( leftover != nullptr )
    {
        // may cause unexpected behaviour if bag_size is too low currently can hold
        // 2^32-1 vertices 
        // in future could extend to dynamic resize to reduce memory usage
        pennants[++largest_both] = leftover;
    }

    largest = largest_both;
}

VertexBag* VertexBag::Split()
{
    /* Splits the give bag into two and returns the other bag */

    VertexBag* other = new VertexBag();
    Pennant* leftover = pennants[0];
    pennants[0] = nullptr;
    
    for(int i = 1; i < largest + 1; i++)
    {
        if( pennants[i] != nullptr )
        {
            other->pennants[i-1] = pennants[i]->Split();
            pennants[i-1] = pennants[i];
            pennants[i] = nullptr;
        }
    }
    if( largest != 0 )
        other->largest = --largest;

    if( leftover != nullptr )
        PushPennant(leftover);
    
    return other;
}

VertexBag::~VertexBag()
{
    for(int i = 0; i < largest + 1; i++)
    {
        if( pennants[i] != nullptr )
        {
            delete pennants[i];
            pennants[i] = nullptr;
        }
    }
}
