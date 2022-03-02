#include <iostream>

struct Node
{
    /* node data structure */

    Node(int _vertex):
    vertex(_vertex),
    left(nullptr),
    right(nullptr) 
    { }

    ~Node()
    {
        // recursively deleting constructor
        // only the root node should be deleted otherwise potential for double free
        if( left != nullptr )
            delete left;
        if( right != nullptr )
            delete right;
    }
    
    int vertex;
    Node* right;
    Node* left;
};
