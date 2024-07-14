#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../include/kdtree.h"

// Forward declarations of static helper functions
static KDTreeNode* kdtree_insert_rec(KDTreeNode* node, double* point, size_t index, size_t depth, size_t dimension);
static void kdtree_destroy_rec(KDTreeNode* node);
static double distance_squared(double* point1, double* point2, size_t dimension);
static void kdtree_nearest_rec(KDTreeNode* node, double* target, KDTreeNode** best, double* best_dist, size_t depth, size_t dimension);

/**
 * Initializes a new KDTree structure.
 * 
 * @param dimension The dimension of the points to be stored in the KDTree.
 * @return Pointer to the newly created KDTree, or NULL on failure.
 */
KDTree* kdtree_create(size_t dimension) {
    KDTree* tree = (KDTree*)malloc(sizeof(KDTree));
    if (!tree) {
        fprintf(stderr, "Failed to allocate memory for KDTree\n");
        return NULL;
    }
    tree->root = NULL;
    tree->dimension = dimension;
    return tree;
}

/**
 * Destroys a KDTree, freeing all associated memory.
 * 
 * @param tree Pointer to the KDTree to be destroyed.
 */
void kdtree_destroy(KDTree* tree) {
    if (tree) {
        kdtree_destroy_rec(tree->root);
        free(tree);
    }
}

/**
 * Frees a KDTree, synonym for kdtree_destroy.
 * 
 * @param tree Pointer to the KDTree to be freed.
 */
void kdtree_free(KDTree* tree) {
    if (tree) {
        kdtree_destroy_rec(tree->root);
        free(tree);
    }
}

/**
 * Recursively destroys a KDTreeNode and its children.
 * 
 * @param node Pointer to the KDTreeNode to be destroyed.
 */
static void kdtree_destroy_rec(KDTreeNode* node) {
    if (node) {
        kdtree_destroy_rec(node->left);
        kdtree_destroy_rec(node->right);
        free(node->point);
        free(node);
    }
}

/**
 * Inserts a new point into the KDTree.
 * 
 * @param tree Pointer to the KDTree.
 * @param point Array of doubles representing the point to be inserted.
 * @param index The index of the point in the original data structure.
 */
void kdtree_insert(KDTree* tree, double* point, size_t index) {
    tree->root = kdtree_insert_rec(tree->root, point, index, 0, tree->dimension);
}

/**
 * Recursively inserts a point into the KDTree.
 * 
 * @param node Current KDTreeNode in the recursive insertion.
 * @param point Array of doubles representing the point to be inserted.
 * @param index The index of the point in the original data structure.
 * @param depth Current depth in the KDTree.
 * @param dimension The dimension of the points in the KDTree.
 * @return Pointer to the updated KDTreeNode.
 */
static KDTreeNode* kdtree_insert_rec(KDTreeNode* node, double* point, size_t index, size_t depth, size_t dimension) {
    if (!node) {
        KDTreeNode* new_node = (KDTreeNode*)malloc(sizeof(KDTreeNode));
        if (!new_node) {
            fprintf(stderr, "Failed to allocate memory for KDTreeNode\n");
            return NULL;
        }
        new_node->point = (double*)malloc(dimension * sizeof(double));
        if (!new_node->point) {
            fprintf(stderr, "Failed to allocate memory for point\n");
            free(new_node);
            return NULL;
        }
        for (size_t i = 0; i < dimension; ++i) {
            new_node->point[i] = point[i];
        }
        new_node->index = index;
        new_node->left = new_node->right = NULL;
        return new_node;
    }

    // Determine the axis to compare at this depth
    size_t axis = depth % dimension;
    if (point[axis] < node->point[axis]) {
        node->left = kdtree_insert_rec(node->left, point, index, depth + 1, dimension);
    } else {
        node->right = kdtree_insert_rec(node->right, point, index, depth + 1, dimension);
    }

    return node;
}

/**
 * Finds the nearest neighbor in the KDTree for a given point.
 * 
 * @param tree Pointer to the KDTree.
 * @param point Array of doubles representing the point to find the nearest neighbor for.
 * @return The index of the nearest neighbor, or (size_t)-1 if the tree is empty.
 */
size_t kdtree_nearest(KDTree* tree, double* point) {
    KDTreeNode* best = NULL;
    double best_dist = INFINITY;
    kdtree_nearest_rec(tree->root, point, &best, &best_dist, 0, tree->dimension);
    return best ? best->index : (size_t)-1;
}

/**
 * Recursively searches for the nearest neighbor in the KDTree.
 * 
 * @param node Current KDTreeNode in the recursive search.
 * @param target Array of doubles representing the target point.
 * @param best Pointer to the best KDTreeNode found so far.
 * @param best_dist Pointer to the best distance found so far.
 * @param depth Current depth in the KDTree.
 * @param dimension The dimension of the points in the KDTree.
 */
static void kdtree_nearest_rec(KDTreeNode* node, double* target, KDTreeNode** best, double* best_dist, size_t depth, size_t dimension) {
    if (!node) {
        return;
    }

    // Calculate the distance from the target to the current node
    double dist = distance_squared(node->point, target, dimension);
    if (dist < *best_dist) {
        *best_dist = dist;
        *best = node;
    }

    // Determine the axis to compare at this depth
    size_t axis = depth % dimension;
    double diff = target[axis] - node->point[axis];
    KDTreeNode* near_node = (diff < 0) ? node->left : node->right;
    KDTreeNode* far_node = (diff < 0) ? node->right : node->left;

    // Recursively search the nearer subtree
    kdtree_nearest_rec(near_node, target, best, best_dist, depth + 1, dimension);

    // If necessary, search the farther subtree
    if (diff * diff < *best_dist) {
        kdtree_nearest_rec(far_node, target, best, best_dist, depth + 1, dimension);
    }
}

/**
 * Calculates the squared Euclidean distance between two points.
 * 
 * @param point1 Array of doubles representing the first point.
 * @param point2 Array of doubles representing the second point.
 * @param dimension The dimension of the points.
 * @return The squared Euclidean distance between the two points.
 */
static double distance_squared(double* point1, double* point2, size_t dimension) {
    double dist = 0;
    for (size_t i = 0; i < dimension; ++i) {
        double diff = point1[i] - point2[i];
        dist += diff * diff;
    }
    return dist;
}
