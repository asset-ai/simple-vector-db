#ifndef KDTREE_H
#define KDTREE_H

#include <stddef.h>

typedef struct KDTreeNode {
    double* point;
    size_t index;
    struct KDTreeNode* left;
    struct KDTreeNode* right;
} KDTreeNode;

typedef struct {
    KDTreeNode* root;
    size_t dimension;
} KDTree;

KDTree* kdtree_create(size_t dimension);
void kdtree_destroy(KDTree* tree);
void kdtree_free(KDTree* tree); // Add this line
void kdtree_insert(KDTree* tree, double* point, size_t index);
size_t kdtree_nearest(KDTree* tree, double* point);

#endif // KDTREE_H
