#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cjson/cJSON.h"

#include "../include/compare_handler.h"
#include "../include/vector_database.h"

typedef struct {
    char *data;
    size_t data_size;
} ConnectionData;

static enum MHD_Result compare_handler_callback(void* cls, struct MHD_Connection* connection,
                                                const char* url, const char* method,
                                                const char* version, const char* upload_data,
                                                size_t* upload_data_size, void** con_cls);

enum MHD_Result compare_handler(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                size_t* upload_data_size, void** con_cls) {
    struct CompareHandlerData* handler_data = (struct CompareHandlerData*)cls;
    return compare_handler_callback(handler_data->db, connection, url, method, version,
                                    upload_data, upload_data_size, con_cls);
}

static enum MHD_Result compare_handler_callback(void* cls, struct MHD_Connection* connection,
                                                const char* url, const char* method,
                                                const char* version, const char* upload_data,
                                                size_t* upload_data_size, void** con_cls) {
    VectorDatabase* db = (VectorDatabase*)cls;
    const char* index1_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index1");
    const char* index2_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index2");

    if (!index1_str || !index2_str) {
        const char* error_msg = "{\"error\": \"Missing 'index1' or 'index2' query parameter\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    size_t index1 = atoi(index1_str);
    size_t index2 = atoi(index2_str);

    if (index1 >= db->size || index2 >= db->size) {
        const char* error_msg = "{\"error\": \"Index out of bounds\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    Vector* vec1 = vector_db_read(db, index1);
    Vector* vec2 = vector_db_read(db, index2);

    if (!vec1 || !vec2) {
        const char* error_msg = "{\"error\": \"Vector not found\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    if (vec1->dimension != vec2->dimension) {
        const char* error_msg = "{\"error\": \"Vectors have different dimensions\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    float result = 0.0;
    const char* key = NULL;
    if (strcmp(url, "/compare/cosine_similarity") == 0) {
        result = cosine_similarity(*vec1, *vec2);
        key = "cosine_similarity";
    } else if (strcmp(url, "/compare/euclidean_distance") == 0) {
        result = euclidean_distance(*vec1, *vec2);
        key = "euclidean_distance";
    } else if (strcmp(url, "/compare/dot_product") == 0) {
        result = dot_product(*vec1, *vec2);
        key = "dot_product";
    } else {
        const char* error_msg = "{\"error\": \"Unknown comparison method\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    cJSON *json_response = cJSON_CreateObject();
    if (json_response == NULL) {
        const char* error_msg = "{\"error\": \"Internal server error\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    cJSON_AddNumberToObject(json_response, key, result);
    char *json_string = cJSON_PrintUnformatted(json_response);
    cJSON_Delete(json_response);

    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(json_string),
                                                                    (void*)json_string, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}

static enum MHD_Result nearest_handler_callback(void* cls, struct MHD_Connection* connection,
                                                const char* url, const char* method,
                                                const char* version, const char* upload_data,
                                                size_t* upload_data_size, void** con_cls);

enum MHD_Result nearest_handler(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                size_t* upload_data_size, void** con_cls) {
    if (*con_cls == NULL) {
        ConnectionData *con_data = (ConnectionData *)malloc(sizeof(ConnectionData));
        if (con_data == NULL) {
            return MHD_NO;
        }
        con_data->data = NULL;
        con_data->data_size = 0;
        *con_cls = (void *)con_data;
        return MHD_YES;
    }

    struct CompareHandlerData* handler_data = (struct CompareHandlerData*)cls;
    return nearest_handler_callback(handler_data->db, connection, url, method, version,
                                    upload_data, upload_data_size, con_cls);
}

static enum MHD_Result nearest_handler_callback(void* cls, struct MHD_Connection* connection,
                                                const char* url, const char* method,
                                                const char* version, const char* upload_data,
                                                size_t* upload_data_size, void** con_cls) {
    ConnectionData *con_data = (ConnectionData *)*con_cls;
    VectorDatabase* db = (VectorDatabase*)cls;

    if (*upload_data_size != 0) {
        con_data->data = (char *)realloc(con_data->data, con_data->data_size + *upload_data_size + 1);
        if (con_data->data == NULL) {
            return MHD_NO;
        }
        memcpy(con_data->data + con_data->data_size, upload_data, *upload_data_size);
        con_data->data_size += *upload_data_size;
        con_data->data[con_data->data_size] = '\0';
        *upload_data_size = 0;
        return MHD_YES;
    }

    if (con_data->data_size == 0) {
        const char* error_msg = "{\"error\": \"Empty data\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    cJSON *json = cJSON_Parse(con_data->data);
    if (json == NULL) {
        const char* error_msg = "{\"error\": \"Invalid JSON\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        free(con_data->data);
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    size_t dimension = cJSON_GetArraySize(json);
    Vector vec;
    vec.dimension = dimension;
    vec.data = (double*)malloc(dimension * sizeof(double));
    if (!vec.data) {
        const char* error_msg = "{\"error\": \"Memory allocation failed\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        cJSON_Delete(json);
        free(con_data->data);
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    for (size_t i = 0; i < dimension; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        if (!cJSON_IsNumber(item)) {
            const char* error_msg = "{\"error\": \"Invalid vector data\"}";
            struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                            (void*)error_msg, MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
            int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
            MHD_destroy_response(response);
            cJSON_Delete(json);
            free(vec.data);
            free(con_data->data);
            free(con_data);
            *con_cls = NULL;
            return ret == MHD_YES ? MHD_YES : MHD_NO;
        }
        vec.data[i] = item->valuedouble;
    }

    vec.median_point = calculate_median(vec.data, vec.dimension);

    size_t count = 1;
    const char* number_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "number");
    if (number_str) {
        count = (size_t)atoi(number_str);
    }

    // Find the nearest vectors
    double* distances = (double*)malloc(db->size * sizeof(double));
    size_t* indices = (size_t*)malloc(db->size * sizeof(size_t));
    size_t valid_count = 0;

    for (size_t i = 0; i < db->size; ++i) {
        if (db->vectors[i].dimension == vec.dimension) {
            distances[valid_count] = fabs(db->vectors[i].median_point - vec.median_point);
            indices[valid_count] = i;
            valid_count++;
        }
    }

    if (valid_count == 0) {
        const char* error_msg = "{\"error\": \"No vectors with matching dimensions\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        free(distances);
        free(indices);
        cJSON_Delete(json);
        free(vec.data);
        free(con_data->data);
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Print debug information for distances and indices
    printf("Distances and indices before sorting:\n");
    for (size_t i = 0; i < valid_count; ++i) {
        printf("Index %zu: distance = %f\n", indices[i], distances[i]);
    }

    // Sort distances and corresponding indices
    for (size_t i = 0; i < valid_count - 1; ++i) {
        for (size_t j = 0; j < valid_count - i - 1; ++j) {
            if (distances[j] > distances[j + 1]) {
                double temp_distance = distances[j];
                distances[j] = distances[j + 1];
                distances[j + 1] = temp_distance;

                size_t temp_index = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp_index;
            }
        }
    }

    // Print debug information for sorted distances and indices
    printf("Distances and indices after sorting:\n");
    for (size_t i = 0; i < valid_count; ++i) {
        printf("Index %zu: distance = %f\n", indices[i], distances[i]);
    }

    // Prepare the JSON response
    cJSON *json_response = cJSON_CreateArray();
    for (size_t i = 0; i < count && i < valid_count; ++i) {
        cJSON *vector_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(vector_obj, "index", indices[i]);
        cJSON *vector_array = cJSON_CreateDoubleArray(db->vectors[indices[i]].data, db->vectors[indices[i]].dimension);
        cJSON_AddItemToObject(vector_obj, "vector", vector_array);
        cJSON_AddNumberToObject(vector_obj, "median_point", db->vectors[indices[i]].median_point);
        cJSON_AddItemToArray(json_response, vector_obj);
    }

    free(distances);
    free(indices);
    cJSON_Delete(json);
    free(vec.data);
    free(con_data->data);
    free(con_data);
    *con_cls = NULL;

    char *response_str = cJSON_PrintUnformatted(json_response);
    cJSON_Delete(json_response);

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str), response_str, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}