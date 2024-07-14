#ifndef KDTREE_H
#define KDTREE_H

#include <stddef.h>

/**
 * @brief Structure representing a KD-tree node.
 */
typedef struct KDTreeNode {
    double *point; /**< Point in the k-dimensional space */
    size_t index; /**< Index of the point in the original dataset */
    struct KDTreeNode *left; /**< Left child node */
    struct KDTreeNode *right; /**< Right child node */
} KDTreeNode;

/**
 * @brief Structure representing a KD-tree.
 */
typedef struct KDTree {
    KDTreeNode *root; /**< Root node of the KD-tree */
    size_t dimension; /**< Dimensionality of the points */
} KDTree;

/**
 * @brief Create a new KD-tree.
 * 
 * @param dimension Dimensionality of the points.
 * @return Pointer to the newly created KD-tree.
 */
KDTree* kdtree_create(size_t dimension);

/**
 * @brief Insert a point into the KD-tree.
 * 
 * @param tree KD-tree into which the point is to be inserted.
 * @param point Point to be inserted.
 * @param index Index of the point in the original dataset.
 */
void kdtree_insert(KDTree* tree, const double* point, size_t index);

/**
 * @brief Free the memory allocated for the KD-tree.
 * 
 * @param tree KD-tree to be freed.
 */
void kdtree_free(KDTree* tree);

/**
 * @brief Find the nearest neighbor in the KD-tree.
 * 
 * @param tree KD-tree to search in.
 * @param point Point to find the nearest neighbor for.
 * @return Index of the nearest neighbor.
 */
size_t kdtree_nearest(KDTree *tree, const double *point);

#endif // KDTREE_H
