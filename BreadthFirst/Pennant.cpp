#include "Pennant.h"

Pennant::Pennant(Node* _root):
root(_root)
{}

Pennant::~Pennant()
{
    delete root;
}

Pennant* Pennant::Union( Pennant*& other )
{
    /* Joins two pennants together */

    if( other == nullptr )
        return nullptr;

    other->root->right = root->left;

    root->left = other->root;
    other->root = nullptr;
    
    // handle memory deletion of other pennant
    delete other;
    other = nullptr;

    return this;
}

Pennant* Pennant::Split()
{   
    /* Splits one pennant into two */
    if( root->left == nullptr )
        return nullptr;
    
    Pennant* other = new Pennant( root->left );
    root->left = other->root->right;
    other->root->right = nullptr;

    return other;
}
