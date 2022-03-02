#include "Node.h"

class Pennant
{
    /* Pennant data structure */
public:
    Pennant( Node* _root );
    ~Pennant();
    Pennant* Union( Pennant*& other );
    Pennant* Split();
    void Sum(Pennant*& other, Pennant*& leftover);
    Node* root;
};
