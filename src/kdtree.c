#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/kdtree.h"

/**
 * @brief Create a new KD-tree node.
 * 
 * @param point Point in the k-dimensional space.
 * @param index Index of the point in the original dataset.
 * @param dimension Dimensionality of the points.
 * @return Pointer to the newly created KD-tree node.
 */
KDTreeNode* kdtree_create_node(const double *point, size_t index, size_t dimension) {
    printf("Creating KDTreeNode with index %zu and dimension %zu\n", index, dimension);
    KDTreeNode *node = (KDTreeNode*)malloc(sizeof(KDTreeNode));
    if (!node) return NULL;

    node->point = (double*)malloc(dimension * sizeof(double));
    if (!node->point) {
        free(node);
        return NULL;
    }

    for (size_t i = 0; i < dimension; i++) {
        node->point[i] = point[i];
    }

    node->index = index;
    node->left = NULL;
    node->right = NULL;

    return node;
}

/**
 * @brief Insert a point into the KD-tree recursively.
 * 
 * @param node Current node in the KD-tree.
 * @param point Point to be inserted.
 * @param index Index of the point in the original dataset.
 * @param depth Current depth in the KD-tree.
 * @param dimension Dimensionality of the points.
 * @return Pointer to the updated KD-tree node.
 */
KDTreeNode* kdtree_insert_rec(KDTreeNode *node, const double *point, size_t index, size_t depth, size_t dimension) {
    printf("Inserting recursively at depth %zu, dimension %zu\n", depth, dimension);
    if (!node) {
        return kdtree_create_node(point, index, dimension);
    }

    size_t cd = depth % dimension;

    if (point[cd] < node->point[cd]) {
        node->left = kdtree_insert_rec(node->left, point, index, depth + 1, dimension);
    } else {
        node->right = kdtree_insert_rec(node->right, point, index, depth + 1, dimension);
    }

    return node;
}

/**
 * @brief Create a new KD-tree.
 * 
 * @param dimension Dimensionality of the points.
 * @return Pointer to the newly created KD-tree.
 */
KDTree* kdtree_create(size_t dimension) {
    KDTree *tree = (KDTree*)malloc(sizeof(KDTree));
    if (!tree) return NULL;

    tree->root = NULL;
    tree->dimension = dimension;

    return tree;
}

/**
 * @brief Insert a point into the KD-tree.
 * 
 * @param tree KD-tree into which the point is to be inserted.
 * @param point Point to be inserted.
 * @param index Index of the point in the original dataset.
 */
void kdtree_insert(KDTree *tree, const double *point, size_t index) {
    printf("Inserting point into KDTree\n");
    tree->root = kdtree_insert_rec(tree->root, point, index, 0, tree->dimension);
}

/**
 * @brief Free the memory allocated for the KD-tree nodes recursively.
 * 
 * @param node Current node in the KD-tree.
 */
void kdtree_free_rec(KDTreeNode *node) {
    if (node) {
        kdtree_free_rec(node->left);
        kdtree_free_rec(node->right);
        free(node->point);
        free(node);
    }
}

/**
 * @brief Free the memory allocated for the KD-tree.
 * 
 * @param tree KD-tree to be freed.
 */
void kdtree_free(KDTree *tree) {
    if (tree) {
        kdtree_free_rec(tree->root);
        free(tree);
    }
}

/**
 * @brief Find the nearest neighbor in the KD-tree recursively.
 * 
 * @param node Current node in the KD-tree.
 * @param point Point to find the nearest neighbor for.
 * @param depth Current depth in the KD-tree.
 * @param dimension Dimensionality of the points.
 * @param best_node Best node found so far.
 * @param best_dist Best distance found so far.
 * @return Pointer to the best KD-tree node found so far.
 */
KDTreeNode* kdtree_nearest_rec(KDTreeNode *node, const double *point, size_t depth, size_t dimension, KDTreeNode *best_node, double *best_dist) {
    if (!node) return best_node;

    double d = 0;
    for (size_t i = 0; i < dimension; i++) {
        d += (node->point[i] - point[i]) * (node->point[i] - point[i]);
    }

    if (d < *best_dist) {
        *best_dist = d;
        best_node = node;
    }

    size_t cd = depth % dimension;
    KDTreeNode *next_node = NULL, *other_node = NULL;

    if (point[cd] < node->point[cd]) {
        next_node = node->left;
        other_node = node->right;
    } else {
        next_node = node->right;
        other_node = node->left;
    }

    best_node = kdtree_nearest_rec(next_node, point, depth + 1, dimension, best_node, best_dist);

    if ((point[cd] - node->point[cd]) * (point[cd] - node->point[cd]) < *best_dist) {
        best_node = kdtree_nearest_rec(other_node, point, depth + 1, dimension, best_node, best_dist);
    }

    return best_node;
}

/**
 * @brief Find the nearest neighbor in the KD-tree.
 * 
 * @param tree KD-tree to search in.
 * @param point Point to find the nearest neighbor for.
 * @return Index of the nearest neighbor.
 */
size_t kdtree_nearest(KDTree *tree, const double *point) {
    double best_dist = INFINITY;
    KDTreeNode *best_node = kdtree_nearest_rec(tree->root, point, 0, tree->dimension, NULL, &best_dist);
    return best_node ? best_node->index : (size_t)-1;
}
