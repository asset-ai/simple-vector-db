#ifndef VECTOR_DATABASE_H
#define VECTOR_DATABASE_H

#include <stddef.h>

typedef struct {
    size_t dimension;
    double* data;
    double median_point;
} Vector;

typedef struct {
    Vector* vectors;
    size_t size;
    size_t capacity;
} VectorDatabase;

VectorDatabase* vector_db_init(size_t initial_capacity);
size_t vector_db_insert(VectorDatabase* db, Vector vec);
Vector* vector_db_read(VectorDatabase* db, size_t index);
void vector_db_update(VectorDatabase* db, size_t index, Vector vec);
void vector_db_delete(VectorDatabase* db, size_t index);
void vector_db_free(VectorDatabase* db);
void vector_db_save(VectorDatabase* db, const char* filename);
VectorDatabase* vector_db_load(const char* filename);

//comparing functions
float cosine_similarity(Vector vec1, Vector vec2);
float euclidean_distance(Vector vec1, Vector vec2);
float dot_product(Vector vec1, Vector vec2); 

// nearest function
double calculate_median(double* data, size_t dimension);
#endif /* VECTOR_DATABASE_H */
