#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vector_database.h"

#define INITIAL_CAPACITY 10

VectorDatabase* vector_db_init(size_t initial_capacity) {
    VectorDatabase* db = (VectorDatabase*)malloc(sizeof(VectorDatabase));
    if (!db) {
        fprintf(stderr, "Failed to allocate memory for database\n");
        return NULL;
    }
    db->vectors = (Vector*)malloc(initial_capacity * sizeof(Vector));
    if (!db->vectors) {
        fprintf(stderr, "Failed to allocate memory for vectors\n");
        free(db);
        return NULL;
    }
    db->size = 0;
    db->capacity = initial_capacity;

    printf("Initialized vector database with capacity %zu\n", initial_capacity);
    return db;
}

void vector_db_insert(VectorDatabase* db, Vector vec) {
    if (vec.dimension == 0 || vec.data == NULL) {
        fprintf(stderr, "Invalid vector data\n");
        return;
    }
    if (db->size == db->capacity) {
        db->capacity *= 2;
        db->vectors = (Vector*)realloc(db->vectors, db->capacity * sizeof(Vector));
        if (!db->vectors) {
            fprintf(stderr, "Failed to reallocate memory for vectors\n");
            return;
        }
    }
    vec.median_point = calculate_median(vec.data, vec.dimension);
    db->vectors[db->size++] = vec;
    printf("Inserted vector at index %zu with dimension %zu and median_point %f\n", db->size - 1, vec.dimension, vec.median_point);
}

Vector* vector_db_read(VectorDatabase* db, size_t index) {
    if (!db) {
        fprintf(stderr, "Database pointer is NULL\n");
        return NULL;
    }
    if (index >= db->size) {
        fprintf(stderr, "Index %zu out of bounds (size: %zu)\n", index, db->size);
        return NULL;
    }
    return &db->vectors[index];
}

void vector_db_update(VectorDatabase* db, size_t index, Vector vec) {
    if (index < db->size) {
        vec.median_point = calculate_median(vec.data, vec.dimension);
        db->vectors[index] = vec;
        printf("Updated vector at index %zu to: (", index);
        for (size_t i = 0; i < vec.dimension; i++) {
            printf("%f", vec.data[i]);
            if (i < vec.dimension - 1) {
                printf(", ");
            }
        }
        printf("), Median Point: %f\n", vec.median_point);
    }
}

void vector_db_delete(VectorDatabase* db, size_t index) {
    if (index < db->size) {
        free(db->vectors[index].data);
        for (size_t i = index; i < db->size - 1; ++i) {
            db->vectors[i] = db->vectors[i + 1];
        }
        db->size--;
        printf("Deleted vector at index %zu, new size: %zu\n", index, db->size);
    }
}

void vector_db_free(VectorDatabase* db) {
    if (db) {
        for (size_t i = 0; i < db->size; ++i) {
            free(db->vectors[i].data);
            printf("Freed data for vector at index %zu\n", i);
        }
        free(db->vectors);
        free(db);
        printf("Freed vector database\n");
    }
}

void vector_db_save(VectorDatabase* db, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    printf("Saving database of size %zu\n", db->size);
    fwrite(&db->size, sizeof(size_t), 1, file);
    for (size_t i = 0; i < db->size; ++i) {
        if (db->vectors[i].dimension == 0 || db->vectors[i].data == NULL) {
            fprintf(stderr, "Invalid vector at index %zu, skipping\n", i);
            continue;
        }
        printf("Saving vector at index %zu with dimension %zu and median point %f\n", i, db->vectors[i].dimension, db->vectors[i].median_point);
        fwrite(&db->vectors[i].dimension, sizeof(size_t), 1, file);
        fwrite(db->vectors[i].data, sizeof(double), db->vectors[i].dimension, file);
        fwrite(&db->vectors[i].median_point, sizeof(double), 1, file); // Save the median point
    }

    fclose(file);
    printf("Database saved to %s\n", filename);
}

VectorDatabase* vector_db_load(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file for reading");
        return NULL;
    }

    VectorDatabase* db = (VectorDatabase*)malloc(sizeof(VectorDatabase));
    if (!db) {
        fclose(file);
        return NULL;
    }

    fread(&db->size, sizeof(size_t), 1, file);
    db->capacity = db->size > INITIAL_CAPACITY ? db->size : INITIAL_CAPACITY;
    db->vectors = (Vector*)malloc(db->capacity * sizeof(Vector));
    if (!db->vectors) {
        free(db);
        fclose(file);
        return NULL;
    }

    for (size_t i = 0; i < db->size; ++i) {
        fread(&db->vectors[i].dimension, sizeof(size_t), 1, file);
        db->vectors[i].data = (double*)malloc(db->vectors[i].dimension * sizeof(double));
        if (!db->vectors[i].data) {
            for (size_t j = 0; j < i; ++j) {
                free(db->vectors[j].data);
            }
            free(db->vectors);
            free(db);
            fclose(file);
            return NULL;
        }
        fread(db->vectors[i].data, sizeof(double), db->vectors[i].dimension, file);
        fread(&db->vectors[i].median_point, sizeof(double), 1, file);
    }

    fclose(file);
    printf("Database loaded from %s\n", filename);
    return db;
}





float cosine_similarity(Vector vec1, Vector vec2) {
    if (vec1.dimension != vec2.dimension) {
        fprintf(stderr, "Vectors have different dimensions\n");
        return -1.0;
    }
    float dot_product = 0.0, norm_a = 0.0, norm_b = 0.0;
    for (size_t i = 0; i < vec1.dimension; i++) {
        dot_product += vec1.data[i] * vec2.data[i];
        norm_a += vec1.data[i] * vec1.data[i];
        norm_b += vec2.data[i] * vec2.data[i];
    }
    return dot_product / (sqrt(norm_a) * sqrt(norm_b));
}

float euclidean_distance(Vector vec1, Vector vec2) {
    if (vec1.dimension != vec2.dimension) {
        fprintf(stderr, "Vectors have different dimensions\n");
        return -1.0;
    }
    float sum = 0.0;
    for (size_t i = 0; i < vec1.dimension; i++) {
        float diff = vec1.data[i] - vec2.data[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

float dot_product(Vector vec1, Vector vec2) {
    if (vec1.dimension != vec2.dimension) {
        fprintf(stderr, "Vectors have different dimensions\n");
        return -1.0;
    }
    float result = 0.0;
    for (size_t i = 0; i < vec1.dimension; i++) {
        result += vec1.data[i] * vec2.data[i];
    }
    return result;
}

double calculate_median(double* data, size_t dimension) {
    // Simple test - need to implement kd-tree later
    if (dimension % 2 == 0) {
        return (data[dimension / 2 - 1] + data[dimension / 2]) / 2.0;
    } else {
        return data[dimension / 2];
    }
}