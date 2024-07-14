#ifndef VECTOR_DATABASE_H
#define VECTOR_DATABASE_H

#include <stddef.h>
#include "kdtree.h"

/**
 * @struct Vector
 * @brief Represents a vector with its data.
 */
typedef struct {
    size_t dimension;      /**< Dimension of the vector */
    double* data;          /**< Array of vector data */
} Vector;

/**
 * @struct VectorDatabase
 * @brief Represents a database of vectors with dynamic resizing and KD-Tree for efficient search.
 */
typedef struct {
    Vector* vectors;       /**< Array of vectors */
    size_t size;           /**< Current size of the vector array */
    size_t capacity;       /**< Current capacity of the vector array */
    KDTree* kdtree;        /**< KD-Tree for efficient search operations */
} VectorDatabase;

/**
 * @brief Initializes a new vector database.
 * 
 * @param initial_capacity Initial capacity of the vector array.
 * @param dimension Dimension of the vectors and KD-Tree.
 * @return Pointer to the initialized VectorDatabase structure.
 */
VectorDatabase* vector_db_init(size_t initial_capacity, size_t dimension);

/**
 * @brief Frees the memory allocated for the vector database.
 * 
 * @param db Pointer to the VectorDatabase structure to be freed.
 */
void vector_db_free(VectorDatabase* db);

/**
 * @brief Inserts a vector into the database.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param vec Vector to be inserted.
 * @return Index of the inserted vector or -1 on failure.
 */
size_t vector_db_insert(VectorDatabase* db, Vector vec);

/**
 * @brief Reads a vector from the database.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param index Index of the vector to be read.
 * @return Pointer to the vector at the specified index or NULL if the index is out of bounds.
 */
Vector* vector_db_read(VectorDatabase* db, size_t index);

/**
 * @brief Updates a vector in the database.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param index Index of the vector to be updated.
 * @param vec New vector data.
 */
void vector_db_update(VectorDatabase* db, size_t index, Vector vec);

/**
 * @brief Deletes a vector from the database.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param index Index of the vector to be deleted.
 */
void vector_db_delete(VectorDatabase* db, size_t index);

/**
 * @brief Saves the database to a file.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param filename Name of the file to save the database to.
 */
void vector_db_save(VectorDatabase* db, const char* filename);

/**
 * @brief Loads the database from a file.
 * 
 * @param filename Name of the file to load the database from.
 * @param dimension Dimension of the vectors and KD-Tree.
 * @return Pointer to the loaded VectorDatabase structure.
 */
VectorDatabase* vector_db_load(const char* filename, size_t dimension);

/**
 * @brief Calculates the cosine similarity between two vectors.
 * 
 * @param vec1 First vector.
 * @param vec2 Second vector.
 * @return Cosine similarity value.
 */
float cosine_similarity(Vector vec1, Vector vec2);

/**
 * @brief Calculates the Euclidean distance between two vectors.
 * 
 * @param vec1 First vector.
 * @param vec2 Second vector.
 * @return Euclidean distance value.
 */
float euclidean_distance(Vector vec1, Vector vec2);

/**
 * @brief Calculates the dot product of two vectors.
 * 
 * @param vec1 First vector.
 * @param vec2 Second vector.
 * @return Dot product value.
 */
float dot_product(Vector vec1, Vector vec2);

#endif // VECTOR_DATABASE_H
