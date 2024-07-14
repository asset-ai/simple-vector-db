#ifndef KDTREE_H
#define KDTREE_H

#include <stddef.h>

// Structure représentant un nœud dans le KDTree
typedef struct KDNode {
    double* point;
    size_t index;
    struct KDNode* left;
    struct KDNode* right;
} KDNode;

// Structure représentant le KDTree
typedef struct KDTree {
    KDNode* root;
    size_t dimension;
} KDTree;

// Fonctions pour manipuler le KDTree
KDTree* kdtree_create(size_t dimension);
void kdtree_insert(KDTree* tree, double* point, size_t index);
size_t kdtree_nearest(KDTree* tree, double* point);
void kdtree_free(KDTree* tree);

#endif // KDTREE_H
