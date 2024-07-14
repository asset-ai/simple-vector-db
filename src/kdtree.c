#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "../include/kdtree.h"

/**
 * @brief Create a new KDTree node.
 * 
 * @param point Point data for the node.
 * @param index Index of the point in the original data structure.
 * @param dimension Dimension of the points.
 * @return KDNode* Pointer to the created KDNode.
 */
KDNode* kdtree_create_node(double* point, size_t index, size_t dimension) {
    KDNode* node = (KDNode*)malloc(sizeof(KDNode));
    if (!node) {
        fprintf(stderr, "kdtree_create_node: Failed to allocate memory for KDNode\n");
        return NULL;
    }
    node->point = (double*)malloc(dimension * sizeof(double));
    if (!node->point) {
        fprintf(stderr, "kdtree_create_node: Failed to allocate memory for node point\n");
        free(node);
        return NULL;
    }
    for (size_t i = 0; i < dimension; ++i) {
        node->point[i] = point[i];
    }
    node->index = index;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/**
 * @brief Insert a point into the KDTree recursively.
 * 
 * @param node Current node in the KDTree.
 * @param point Point to be inserted.
 * @param index Index of the point in the original data structure.
 * @param depth Current depth in the KDTree.
 * @param dimension Dimension of the points.
 * @return KDNode* Pointer to the root of the KDTree.
 */
KDNode* kdtree_insert_rec(KDNode* node, double* point, size_t index, size_t depth, size_t dimension) {
    if (!node) {
        return kdtree_create_node(point, index, dimension);
    }

    size_t cd = depth % dimension;

    // Recur down the tree
    if (point[cd] < node->point[cd]) {
        node->left = kdtree_insert_rec(node->left, point, index, depth + 1, dimension);
    } else {
        node->right = kdtree_insert_rec(node->right, point, index, depth + 1, dimension);
    }

    return node;
}

/**
 * @brief Create a KDTree with the given dimension.
 * 
 * @param dimension Dimension of the points.
 * @return KDTree* Pointer to the created KDTree.
 */
KDTree* kdtree_create(size_t dimension) {
    KDTree* tree = (KDTree*)malloc(sizeof(KDTree));
    if (!tree) {
        fprintf(stderr, "kdtree_create: Failed to allocate memory for KDTree\n");
        return NULL;
    }
    tree->root = NULL;
    tree->dimension = dimension;
    return tree;
}

/**
 * @brief Insert a point into the KDTree.
 * 
 * @param tree Pointer to the KDTree.
 * @param point Point to be inserted.
 * @param index Index of the point in the original data structure.
 */
void kdtree_insert(KDTree* tree, double* point, size_t index) {
    if (!tree) {
        fprintf(stderr, "kdtree_insert: KDTree is NULL\n");
        return;
    }
    printf("Inserting point into KDTree\n");
    tree->root = kdtree_insert_rec(tree->root, point, index, 0, tree->dimension);
}

/**
 * @brief Recursively find the nearest neighbor in the KDTree.
 * 
 * @param node Current node in the KDTree.
 * @param point Target point.
 * @param depth Current depth in the KDTree.
 * @param dimension Dimension of the points.
 * @param best_node Best node found so far.
 * @param best_dist Best distance found so far.
 * @return KDNode* Pointer to the nearest neighbor node.
 */
KDNode* kdtree_nearest_rec(KDNode* node, double* point, size_t depth, size_t dimension, KDNode* best_node, double* best_dist) {
    if (!node) {
        return best_node;
    }

    double d = 0.0;
    for (size_t i = 0; i < dimension; ++i) {
        d += (node->point[i] - point[i]) * (node->point[i] - point[i]);
    }

    if (d < *best_dist) {
        *best_dist = d;
        best_node = node;
    }

    size_t cd = depth % dimension;
    KDNode* next_node = (point[cd] < node->point[cd]) ? node->left : node->right;
    KDNode* other_node = (point[cd] < node->point[cd]) ? node->right : node->left;

    best_node = kdtree_nearest_rec(next_node, point, depth + 1, dimension, best_node, best_dist);
    if ((point[cd] - node->point[cd]) * (point[cd] - node->point[cd]) < *best_dist) {
        best_node = kdtree_nearest_rec(other_node, point, depth + 1, dimension, best_node, best_dist);
    }

    return best_node;
}

/**
 * @brief Find the nearest neighbor in the KDTree.
 * 
 * @param tree Pointer to the KDTree.
 * @param point Target point.
 * @return size_t Index of the nearest neighbor.
 */
size_t kdtree_nearest(KDTree* tree, double* point) {
    if (!tree || !tree->root) {
        fprintf(stderr, "kdtree_nearest: KDTree is NULL or empty\n");
        return (size_t)-1;
    }

    double best_dist = DBL_MAX;
    KDNode* best_node = kdtree_nearest_rec(tree->root, point, 0, tree->dimension, NULL, &best_dist);

    return best_node ? best_node->index : (size_t)-1;
}

/**
 * @brief Free the memory allocated for a KDTree node.
 * 
 * @param node Pointer to the KDTree node to be freed.
 */
void kdtree_free_node(KDNode* node) {
    if (node) {
        free(node->point);
        kdtree_free_node(node->left);
        kdtree_free_node(node->right);
        free(node);
    }
}

/**
 * @brief Free the memory allocated for the KDTree.
 * 
 * @param tree Pointer to the KDTree to be freed.
 */
void kdtree_free(KDTree* tree) {
    if (tree) {
        kdtree_free_node(tree->root);
        free(tree);
    }
}
