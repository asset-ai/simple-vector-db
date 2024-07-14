#ifndef KDTREE_H
#define KDTREE_H

#include <stddef.h>

/**
 * @struct KDTreeNode
 * @brief Structure representing a node in the KD-Tree.
 */
typedef struct KDTreeNode {
    double* point;         /**< Array of double values representing the point coordinates */
    size_t index;          /**< Index of the point in the original dataset */
    struct KDTreeNode* left;  /**< Pointer to the left child node */
    struct KDTreeNode* right; /**< Pointer to the right child node */
} KDTreeNode;

/**
 * @struct KDTree
 * @brief Structure representing a KD-Tree.
 */
typedef struct {
    KDTreeNode* root;     /**< Pointer to the root node of the KD-Tree */
    size_t dimension;     /**< Dimension of the points in the KD-Tree */
} KDTree;

/**
 * @brief Creates a new KD-Tree.
 * 
 * @param dimension Dimension of the points to be stored in the KD-Tree.
 * @return Pointer to the created KDTree structure, or NULL if allocation fails.
 */
KDTree* kdtree_create(size_t dimension);

/**
 * @brief Destroys a KD-Tree and frees the associated memory.
 * 
 * @param tree Pointer to the KDTree structure to be destroyed.
 */
void kdtree_destroy(KDTree* tree);

/**
 * @brief Frees the memory allocated for a KD-Tree.
 * 
 * @param tree Pointer to the KDTree structure to be freed.
 */
void kdtree_free(KDTree* tree);

/**
 * @brief Inserts a point into the KD-Tree.
 * 
 * @param tree Pointer to the KDTree structure.
 * @param point Array of double values representing the point coordinates.
 * @param index Index of the point in the original dataset.
 */
void kdtree_insert(KDTree* tree, double* point, size_t index);

/**
 * @brief Finds the nearest point in the KD-Tree to a given point.
 * 
 * @param tree Pointer to the KDTree structure.
 * @param point Array of double values representing the point coordinates to search for.
 * @return Index of the nearest point in the KD-Tree.
 */
size_t kdtree_nearest(KDTree* tree, double* point);

#endif // KDTREE_H
