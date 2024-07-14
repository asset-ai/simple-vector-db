#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../include/kdtree.h"

static KDTreeNode* kdtree_insert_rec(KDTreeNode* node, double* point, size_t index, size_t depth, size_t dimension);
static void kdtree_destroy_rec(KDTreeNode* node);
static double distance_squared(double* point1, double* point2, size_t dimension);
static void kdtree_nearest_rec(KDTreeNode* node, double* target, KDTreeNode** best, double* best_dist, size_t depth, size_t dimension);

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

void kdtree_destroy(KDTree* tree) {
    if (tree) {
        kdtree_destroy_rec(tree->root);
        free(tree);
    }
}

void kdtree_free(KDTree* tree) {
    if (tree) {
        kdtree_destroy_rec(tree->root);
        free(tree);
    }
}

static void kdtree_destroy_rec(KDTreeNode* node) {
    if (node) {
        kdtree_destroy_rec(node->left);
        kdtree_destroy_rec(node->right);
        free(node->point);
        free(node);
    }
}

void kdtree_insert(KDTree* tree, double* point, size_t index) {
    tree->root = kdtree_insert_rec(tree->root, point, index, 0, tree->dimension);
}

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

    size_t axis = depth % dimension;
    if (point[axis] < node->point[axis]) {
        node->left = kdtree_insert_rec(node->left, point, index, depth + 1, dimension);
    } else {
        node->right = kdtree_insert_rec(node->right, point, index, depth + 1, dimension);
    }

    return node;
}

size_t kdtree_nearest(KDTree* tree, double* point) {
    KDTreeNode* best = NULL;
    double best_dist = INFINITY;
    kdtree_nearest_rec(tree->root, point, &best, &best_dist, 0, tree->dimension);
    return best ? best->index : (size_t)-1;
}

static void kdtree_nearest_rec(KDTreeNode* node, double* target, KDTreeNode** best, double* best_dist, size_t depth, size_t dimension) {
    if (!node) {
        return;
    }

    double dist = distance_squared(node->point, target, dimension);
    if (dist < *best_dist) {
        *best_dist = dist;
        *best = node;
    }

    size_t axis = depth % dimension;
    double diff = target[axis] - node->point[axis];
    KDTreeNode* near_node = (diff < 0) ? node->left : node->right;
    KDTreeNode* far_node = (diff < 0) ? node->right : node->left;

    kdtree_nearest_rec(near_node, target, best, best_dist, depth + 1, dimension);
    if (diff * diff < *best_dist) {
        kdtree_nearest_rec(far_node, target, best, best_dist, depth + 1, dimension);
    }
}

static double distance_squared(double* point1, double* point2, size_t dimension) {
    double dist = 0;
    for (size_t i = 0; i < dimension; ++i) {
        double diff = point1[i] - point2[i];
        dist += diff * diff;
    }
    return dist;
}
