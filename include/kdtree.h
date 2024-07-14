#ifndef KDTREE_H
#define KDTREE_H

#include <stddef.h>

/**
 * @brief Structure representing a node in the KDTree.
 */
typedef struct KDNode {
    double* point;          /**< Pointer to the coordinates of the point */
    size_t index;           /**< Index of the point */
    struct KDNode* left;    /**< Pointer to the left child node */
    struct KDNode* right;   /**< Pointer to the right child node */
} KDNode;

/**
 * @brief Structure representing the KDTree.
 */
typedef struct KDTree {
    KDNode* root;           /**< Pointer to the root node of the tree */
    size_t dimension;       /**< Dimensionality of the KDTree */
} KDTree;

/**
 * @brief Creates a KDTree with the specified dimensionality.
 * 
 * @param dimension The dimensionality of the KDTree.
 * @return Pointer to the created KDTree.
 */
KDTree* kdtree_create(size_t dimension);

/**
 * @brief Inserts a point into the KDTree.
 * 
 * @param tree Pointer to the KDTree.
 * @param point Pointer to the coordinates of the point to insert.
 * @param index Index of the point.
 */
void kdtree_insert(KDTree* tree, double* point, size_t index);

/**
 * @brief Finds the index of the nearest neighbor to a given point in the KDTree.
 * 
 * @param tree Pointer to the KDTree.
 * @param point Pointer to the coordinates of the query point.
 * @return Index of the nearest neighbor point.
 */
size_t kdtree_nearest(KDTree* tree, double* point);

/**
 * @brief Frees the memory allocated for the KDTree.
 * 
 * @param tree Pointer to the KDTree to free.
 */
void kdtree_free(KDTree* tree);

#endif // KDTREE_H
