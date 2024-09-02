#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cjson/cJSON.h"

#include "../include/compare_handler.h"
#include "../include/vector_database.h"
#include "../include/connection_data.h"

/**
 * @struct PostHandlerData
 * @brief Structure to hold data for the POST handler
 */
typedef struct {
    VectorDatabase* db;
    size_t db_vector_size;
} PostHandlerData;

/**
 * @brief Callback function to handle comparison requests.
 * 
 * @param cls User-defined data, in this case, the handler data.
 * @param connection Pointer to MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "GET").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result compare_handler_callback(void* cls, struct MHD_Connection* connection,
                                                const char* url, const char* method,
                                                const char* version, const char* upload_data,
                                                size_t* upload_data_size, void** con_cls);

/**
 * @brief Function to handle comparison requests.
 * 
 * @param cls User-defined data, in this case, the handler data.
 * @param connection Pointer to MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "GET").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
enum MHD_Result compare_handler(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                size_t* upload_data_size, void** con_cls) {
    PostHandlerData* handler_data = (PostHandlerData*)cls;
    return compare_handler_callback(handler_data, connection, url, method, version,
                                    upload_data, upload_data_size, con_cls);
}

/**
 * @brief Callback function to handle comparison requests.
 * 
 * @param cls User-defined data, in this case, the handler data.
 * @param connection Pointer to MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "GET").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result compare_handler_callback(void* cls, struct MHD_Connection* connection,
                                                const char* url, const char* method,
                                                const char* version, const char* upload_data,
                                                size_t* upload_data_size, void** con_cls) {
    PostHandlerData* handler_data = (PostHandlerData*)cls;
    VectorDatabase* db = handler_data->db;
    size_t expected_vector_size = handler_data->db_vector_size;

    // Retrieve 'index1' and 'index2' query parameters
    const char* index1_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index1");
    const char* index2_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index2");

    if (!index1_str || !index2_str) {
        // Respond with an error if 'index1' or 'index2' is missing
        const char* error_msg = "{\"error\": \"Missing 'index1' or 'index2' query parameter\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Convert 'index1' and 'index2' to size_t values
    size_t index1 = atoi(index1_str);
    size_t index2 = atoi(index2_str);

    if (index1 >= db->size || index2 >= db->size) {
        // Respond with an error if the indices are out of bounds
        const char* error_msg = "{\"error\": \"Index out of bounds\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Retrieve the vectors from the database
    Vector* vec1 = vector_db_read(db, index1);
    Vector* vec2 = vector_db_read(db, index2);

    if (!vec1 || !vec2) {
        // Respond with an error if one of the vectors is not found
        const char* error_msg = "{\"error\": \"Vector not found\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    if (vec1->dimension != vec2->dimension) {
        // Respond with an error if the vectors have different dimensions
        const char* error_msg = "{\"error\": \"Vectors have different dimensions\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Check if the vectors match the expected size
    if (vec1->dimension != expected_vector_size || vec2->dimension != expected_vector_size) {
        const char* error_msg = "{\"error\": \"Vector size mismatch\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Perform the comparison based on the URL
    double result = 0.0;
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
        // Respond with an error if the comparison method is unknown
        const char* error_msg = "{\"error\": \"Unknown comparison method\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Create the JSON response
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

    // Send the JSON response
    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(json_string),
                                                                    (void*)json_string, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}

/**
 * @brief Callback function to handle nearest neighbor requests.
 * 
 * @param cls User-defined data, in this case, the handler data.
 * @param connection Pointer to MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "POST").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result nearest_handler_callback(void* cls, struct MHD_Connection* connection,
                                                const char* url, const char* method,
                                                const char* version, const char* upload_data,
                                                size_t* upload_data_size, void** con_cls);

/**
 * @brief Function to handle nearest neighbor requests.
 * 
 * @param cls User-defined data, in this case, the handler data.
 * @param connection Pointer to MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "POST").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
enum MHD_Result nearest_handler(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                size_t* upload_data_size, void** con_cls) {
    // Initialize connection data if it's the first call for this connection
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

    // Retrieve the handler data
    PostHandlerData* handler_data = (PostHandlerData*)cls;
    return nearest_handler_callback(handler_data, connection, url, method, version,
                                    upload_data, upload_data_size, con_cls);
}

/**
 * @brief Callback function to handle nearest neighbor requests.
 * 
 * @param cls User-defined data, in this case, the handler data.
 * @param connection Pointer to MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "POST").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result nearest_handler_callback(void* cls, struct MHD_Connection* connection,
                                                const char* url, const char* method,
                                                const char* version, const char* upload_data,
                                                size_t* upload_data_size, void** con_cls) {
    ConnectionData *con_data = (ConnectionData *)*con_cls;
    PostHandlerData* handler_data = (PostHandlerData*)cls;
    VectorDatabase* db = handler_data->db;
    size_t expected_vector_size = handler_data->db_vector_size;

    // Check if there's data to be uploaded
    if (*upload_data_size != 0) {
        // Reallocate memory to accommodate the new data
        con_data->data = (char *)realloc(con_data->data, con_data->data_size + *upload_data_size + 1);
        if (con_data->data == NULL) {
            return MHD_NO;
        }
        // Copy the upload data to the connection data buffer
        memcpy(con_data->data + con_data->data_size, upload_data, *upload_data_size);
        con_data->data_size += *upload_data_size;
        con_data->data[con_data->data_size] = '\0'; // Null-terminate the data
        *upload_data_size = 0; // Reset the upload data size
        return MHD_YES;
    }

    // Check if the connection data buffer is empty
    if (con_data->data_size == 0) {
        // Respond with an error if there's no data
        const char* error_msg = "{\"error\": \"Empty data\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Parse the JSON data from the request
    cJSON *json = cJSON_Parse(con_data->data);
    if (json == NULL) {
        // Respond with an error if the JSON is invalid
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

    // Get the dimension of the vector from the JSON array size
    size_t dimension = cJSON_GetArraySize(json);
    printf("nearest_handler_callback: Vector dimension: %zu\n", dimension);

    // Check if the vector size matches the expected size
    if (dimension != expected_vector_size) {
        fprintf(stderr, "nearest_handler_callback: Vector size mismatch. Expected %zu, got %zu\n", expected_vector_size, dimension);
        const char* error_msg = "{\"error\": \"Vector size mismatch\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            fprintf(stderr, "nearest_handler_callback: Failed to create response\n");
            cJSON_Delete(json);
            free(con_data->data);
            free(con_data);
            *con_cls = NULL;
            return MHD_NO;
        }
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        cJSON_Delete(json);
        free(con_data->data);
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    Vector vec;
    vec.dimension = dimension;
    // Allocate memory for the vector data
    vec.data = (double*)malloc(dimension * sizeof(double));
    if (!vec.data) {
        // Respond with an error if memory allocation fails
        const char* error_msg = "{\"error\": \"Memory allocation failed\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            fprintf(stderr, "nearest_handler_callback: Failed to create response\n");
            cJSON_Delete(json);
            free(con_data->data);
            free(con_data);
            *con_cls = NULL;
            return MHD_NO;
        }
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        cJSON_Delete(json);
        free(con_data->data);
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Extract the vector data from the JSON array
    for (size_t i = 0; i < dimension; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        if (!cJSON_IsNumber(item)) {
            // Respond with an error if the vector data is invalid
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
        // Store the vector data
        vec.data[i] = item->valuedouble;
    }

    // Debug: Print out the vector
    printf("Received vector for nearest neighbor search: [");
    for (size_t i = 0; i < dimension; ++i) {
        printf("%f", vec.data[i]);
        if (i < dimension - 1) {
            printf(", ");
        }
    }
    printf("]\n");

    // Use KD-Tree to find the nearest neighbor
    size_t nearest_index = kdtree_nearest(db->kdtree, vec.data);

    // Debug: Print the nearest index
    printf("Nearest neighbor index: %zu\n", nearest_index);

    // Create the JSON response
    cJSON* json_response = cJSON_CreateObject();
    if (nearest_index != (size_t)-1) {
        Vector* nearest_vector = vector_db_read(db, nearest_index);
        if (nearest_vector) {
            cJSON* vector_array = cJSON_CreateDoubleArray(nearest_vector->data, nearest_vector->dimension);
            cJSON_AddNumberToObject(json_response, "index", nearest_index);
            cJSON_AddItemToObject(json_response, "vector", vector_array);
            cJSON_AddStringToObject(json_response, "uuid", nearest_vector->uuid);
        } else {
            cJSON_AddStringToObject(json_response, "error", "Nearest neighbor not found");
        }
    } else {
        cJSON_AddStringToObject(json_response, "error", "No nearest neighbor found");
    }

    // Convert the JSON response to a string
    char* response_str = cJSON_PrintUnformatted(json_response);
    cJSON_Delete(json_response);

    // Free allocated memory
    free(vec.data);
    free(con_data->data);
    free(con_data);
    *con_cls = NULL;

    // Send the JSON response
    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void*)response_str, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}
