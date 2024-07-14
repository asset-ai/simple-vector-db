#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../include/vector_database.h"
#include "../include/kdtree.h"

/**
 * @brief Initializes the vector database.
 * 
 * @param initial_capacity Initial capacity of the vector array.
 * @param dimension Dimension of the vectors and KD-Tree.
 * @return Pointer to the initialized VectorDatabase structure.
 */
VectorDatabase* vector_db_init(size_t initial_capacity, size_t dimension) {
    VectorDatabase* db = (VectorDatabase*)malloc(sizeof(VectorDatabase));
    if (!db) {
        fprintf(stderr, "Failed to allocate memory for database\n");
        return NULL;
    }

    db->size = 0;
    db->capacity = initial_capacity > 0 ? initial_capacity : 10;
    db->vectors = (Vector*)malloc(db->capacity * sizeof(Vector));
    if (!db->vectors) {
        fprintf(stderr, "Failed to allocate memory for vectors\n");
        free(db);
        return NULL;
    }

    db->kdtree = kdtree_create(dimension);
    if (!db->kdtree) {
        fprintf(stderr, "Failed to create KDTree\n");
        free(db->vectors);
        free(db);
        return NULL;
    } else {
        printf("KDTree initialized\n");
    }
    return db;
}

/**
 * @brief Frees the memory allocated for the vector database.
 * 
 * @param db Pointer to the VectorDatabase structure to be freed.
 */
void vector_db_free(VectorDatabase* db) {
    if (db) {
        // Free memory for each vector's data
        for (size_t i = 0; i < db->size; ++i) {
            free(db->vectors[i].data);
        }
        // Free the KD-Tree
        kdtree_free(db->kdtree);
        // Free the vector array and the VectorDatabase structure
        free(db->vectors);
        free(db);
    }
}

/**
 * @brief Inserts a vector into the database.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param vec Vector to be inserted.
 * @return Index of the inserted vector or -1 on failure.
 */
size_t vector_db_insert(VectorDatabase* db, Vector vec) {
    if (db->size >= db->capacity) {
        db->capacity *= 2;
        Vector* new_vectors = (Vector*)realloc(db->vectors, db->capacity * sizeof(Vector));
        if (!new_vectors) {
            fprintf(stderr, "Failed to allocate more memory for vectors\n");
            return (size_t)-1;
        }
        db->vectors = new_vectors;
    }
    if (!db->kdtree) {
        fprintf(stderr, "KDTree is NULL before inserting\n");
        return (size_t)-1;
    }
    db->vectors[db->size] = vec;
    kdtree_insert(db->kdtree, vec.data, db->size);
    printf("Inserted vector at index %zu with dimension %zu\n", db->size, vec.dimension);
    return db->size++;
}

/**
 * @brief Reads a vector from the database.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param index Index of the vector to be read.
 * @return Pointer to the vector at the specified index or NULL if the index is out of bounds.
 */
Vector* vector_db_read(VectorDatabase* db, size_t index) {
    if (index < db->size) {
        return &db->vectors[index];
    }
    return NULL;
}

/**
 * @brief Updates a vector in the database.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param index Index of the vector to be updated.
 * @param vec New vector data.
 */
void vector_db_update(VectorDatabase* db, size_t index, Vector vec) {
    if (index < db->size) {
        // Free the old vector data
        free(db->vectors[index].data);
        db->vectors[index] = vec;
        // Insert the updated vector into the KD-Tree
        kdtree_insert(db->kdtree, vec.data, index);
    }
}

/**
 * @brief Deletes a vector from the database.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param index Index of the vector to be deleted.
 */
void vector_db_delete(VectorDatabase* db, size_t index) {
    if (index < db->size) {
        // Free the vector data
        free(db->vectors[index].data);
        // Shift the vectors to fill the gap
        for (size_t i = index; i < db->size - 1; ++i) {
            db->vectors[i] = db->vectors[i + 1];
        }
        db->size--;
    }
}

/**
 * @brief Saves the database to a file.
 * 
 * @param db Pointer to the VectorDatabase structure.
 * @param filename Name of the file to save the database to.
 */
void vector_db_save(VectorDatabase* db, const char* filename) {
    // Open the file for writing
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    printf("Saving database of size %zu\n", db->size);
    // Write the size of the database
    fwrite(&db->size, sizeof(size_t), 1, file);
    // Write each vector to the file
    for (size_t i = 0; i < db->size; ++i) {
        if (db->vectors[i].dimension == 0 || db->vectors[i].data == NULL) {
            fprintf(stderr, "Invalid vector at index %zu, skipping\n", i);
            continue;
        }
        printf("Saving vector at index %zu with dimension %zu\n", i, db->vectors[i].dimension);
        fwrite(&db->vectors[i].dimension, sizeof(size_t), 1, file);
        fwrite(db->vectors[i].data, sizeof(double), db->vectors[i].dimension, file);
    }

    fclose(file);
    printf("Database saved to %s\n", filename);
}

/**
 * @brief Loads the database from a file.
 * 
 * @param filename Name of the file to load the database from.
 * @param dimension Dimension of the vectors and KD-Tree.
 * @return Pointer to the loaded VectorDatabase structure.
 */
VectorDatabase* vector_db_load(const char* filename, size_t dimension) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file for reading");
        return NULL;
    }
    VectorDatabase* db = (VectorDatabase*)malloc(sizeof(VectorDatabase));
    if (!db) {
        fprintf(stderr, "Failed to allocate memory for database\n");
        fclose(file);
        return NULL;
    }
    fread(&db->size, sizeof(size_t), 1, file);
    db->capacity = db->size > 0 ? db->size : 10;
    db->vectors = (Vector*)malloc(db->capacity * sizeof(Vector));
    if (!db->vectors) {
        fprintf(stderr, "Failed to allocate memory for vectors\n");
        free(db);
        fclose(file);
        return NULL;
    }
    for (size_t i = 0; i < db->size; ++i) {
        fread(&db->vectors[i].dimension, sizeof(size_t), 1, file);
        db->vectors[i].data = (double*)malloc(db->vectors[i].dimension * sizeof(double));
        if (!db->vectors[i].data) {
            fprintf(stderr, "Failed to allocate memory for vector data\n");
            vector_db_free(db);
            fclose(file);
            return NULL;
        }
        fread(db->vectors[i].data, sizeof(double), db->vectors[i].dimension, file);
    }
    db->kdtree = kdtree_create(dimension);
    if (!db->kdtree) {
        fprintf(stderr, "Failed to create KDTree\n");
        vector_db_free(db);
        fclose(file);
        return NULL;
    }
    for (size_t i = 0; i < db->size; ++i) {
        kdtree_insert(db->kdtree, db->vectors[i].data, i);
    }
    fclose(file);
    return db;
}

/**
 * @brief Calculates the cosine similarity between two vectors.
 * 
 * @param vec1 First vector.
 * @param vec2 Second vector.
 * @return Cosine similarity value.
 */
float cosine_similarity(Vector vec1, Vector vec2) {
    if (vec1.dimension != vec2.dimension) {
        fprintf(stderr, "Vectors have different dimensions\n");
        return -1.0;
    }
    float dot_product = 0.0, norm_a = 0.0, norm_b = 0.0;
    // Calculate the dot product and norms
    for (size_t i = 0; i < vec1.dimension; i++) {
        dot_product += vec1.data[i] * vec2.data[i];
        norm_a += vec1.data[i] * vec1.data[i];
        norm_b += vec2.data[i] * vec2.data[i];
    }
    // Return the cosine similarity
    return dot_product / (sqrt(norm_a) * sqrt(norm_b));
}

/**
 * @brief Calculates the Euclidean distance between two vectors.
 * 
 * @param vec1 First vector.
 * @param vec2 Second vector.
 * @return Euclidean distance value.
 */
float euclidean_distance(Vector vec1, Vector vec2) {
    if (vec1.dimension != vec2.dimension) {
        fprintf(stderr, "Vectors have different dimensions\n");
        return -1.0;
    }
    float sum = 0.0;
    // Calculate the sum of squared differences
    for (size_t i = 0; i < vec1.dimension; i++) {
        float diff = vec1.data[i] - vec2.data[i];
        sum += diff * diff;
    }
    // Return the Euclidean distance
    return sqrt(sum);
}

/**
 * @brief Calculates the dot product of two vectors.
 * 
 * @param vec1 First vector.
 * @param vec2 Second vector.
 * @return Dot product value.
 */
float dot_product(Vector vec1, Vector vec2) {
    if (vec1.dimension != vec2.dimension) {
        fprintf(stderr, "Vectors have different dimensions\n");
        return -1.0;
    }
    float result = 0.0;
    // Calculate the dot product
    for (size_t i = 0; i < vec1.dimension; i++) {
        result += vec1.data[i] * vec2.data[i];
    }
    return result;
}

/**
 * @brief Compares two double values for qsort.
 * 
 * @param a Pointer to the first double value.
 * @param b Pointer to the second double value.
 * @return Comparison result: -1 if a < b, 1 if a > b, 0 if a == b.
 */
int compare(const void* a, const void* b) {
    double arg1 = *(const double*)a;
    double arg2 = *(const double*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}
