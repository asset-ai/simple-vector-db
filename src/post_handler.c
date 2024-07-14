#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include "cjson/cJSON.h"
#include "../include/vector_database.h"

// Structure to hold connection data
typedef struct {
    char *data;
    size_t data_size;
} ConnectionData;

/**
 * @brief Callback function to handle the POST request data.
 * 
 * @param cls User-defined data, in this case, the database.
 * @param connection MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "POST").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result post_handler_callback(void* cls, struct MHD_Connection* connection,
                                            const char* url, const char* method,
                                            const char* version, const char* upload_data,
                                            size_t* upload_data_size, void** con_cls);

// Structure to hold data for the POST handler
struct PostHandlerData {
    VectorDatabase* db;
};

/**
 * @brief Function to handle POST requests.
 * 
 * @param cls User-defined data, in this case, the database.
 * @param connection MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "POST").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
enum MHD_Result post_handler(void* cls, struct MHD_Connection* connection,
                             const char* url, const char* method,
                             const char* version, const char* upload_data,
                             size_t* upload_data_size, void** con_cls) {
    printf("post_handler: Entered\n");

    // Initialize connection data if it's the first call for this connection
    if (*con_cls == NULL) {
        ConnectionData *con_data = (ConnectionData *)malloc(sizeof(ConnectionData));
        if (con_data == NULL) {
            fprintf(stderr, "post_handler: Failed to allocate memory for ConnectionData\n");
            return MHD_NO;
        }
        con_data->data = NULL;
        con_data->data_size = 0;
        *con_cls = (void *)con_data;
        printf("post_handler: Initialized ConnectionData\n");
        return MHD_YES;
    }

    // Retrieve the handler data
    struct PostHandlerData* handler_data = (struct PostHandlerData*)cls;
    printf("post_handler: Retrieved handler_data\n");
    return post_handler_callback(handler_data->db, connection, url, method, version,
                                 upload_data, upload_data_size, con_cls);
}

/**
 * @brief Callback function to handle POST request data.
 * 
 * @param cls User-defined data, in this case, the database.
 * @param connection MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "POST").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result post_handler_callback(void* cls, struct MHD_Connection* connection,
                                            const char* url, const char* method,
                                            const char* version, const char* upload_data,
                                            size_t* upload_data_size, void** con_cls) {
    printf("post_handler_callback: Entered\n");

    ConnectionData *con_data = (ConnectionData *)*con_cls;
    VectorDatabase* db = (VectorDatabase*)cls;
    if (!db) {
        fprintf(stderr, "post_handler_callback: VectorDatabase is NULL\n");
        return MHD_NO;
    }

    // Check if there's data to be uploaded
    if (*upload_data_size != 0) {
        printf("post_handler_callback: Receiving upload data of size %zu\n", *upload_data_size);
        // Reallocate memory to accommodate the new data
        char *new_data = (char *)realloc(con_data->data, con_data->data_size + *upload_data_size + 1);
        if (new_data == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to reallocate memory for data\n");
            return MHD_NO;
        }
        con_data->data = new_data;
        // Copy the upload data to the connection data buffer
        memcpy(con_data->data + con_data->data_size, upload_data, *upload_data_size);
        con_data->data_size += *upload_data_size;
        con_data->data[con_data->data_size] = '\0'; // Null-terminate the data
        *upload_data_size = 0; // Reset the upload data size
        return MHD_YES;
    }

    // Check if the connection data buffer is empty
    if (con_data->data_size == 0) {
        fprintf(stderr, "post_handler_callback: Empty data received\n");
        const char* error_msg = "{\"error\": \"Empty data\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        printf("post_handler_callback: Freeing con_data->data\n");
        free(con_data->data);
        printf("post_handler_callback: Freeing con_data\n");
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    printf("post_handler_callback: Parsing JSON data\n");

    // Parse the JSON data from the request
    cJSON *json = cJSON_Parse(con_data->data);
    if (json == NULL) {
        fprintf(stderr, "post_handler_callback: Invalid JSON received\n");
        const char* error_msg = "{\"error\": \"Invalid JSON\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        printf("post_handler_callback: Freeing con_data->data\n");
        free(con_data->data);
        printf("post_handler_callback: Freeing con_data\n");
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    printf("post_handler_callback: JSON parsed successfully\n");

    // Get the dimension of the vector from the JSON array size
    size_t dimension = cJSON_GetArraySize(json);
    printf("post_handler_callback: Vector dimension: %zu\n", dimension);
    Vector vec;
    vec.dimension = dimension;
    // Allocate memory for the vector data
    vec.data = (double*)malloc(dimension * sizeof(double));
    if (!vec.data) {
        fprintf(stderr, "post_handler_callback: Memory allocation for vector data failed\n");
        const char* error_msg = "{\"error\": \"Memory allocation failed\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        cJSON_Delete(json);
        printf("post_handler_callback: Freeing con_data->data\n");
        free(con_data->data);
        printf("post_handler_callback: Freeing con_data\n");
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Extract the vector data from the JSON array
    for (size_t i = 0; i < dimension; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        if (!cJSON_IsNumber(item)) {
            fprintf(stderr, "post_handler_callback: Invalid vector data at index %zu\n", i);
            const char* error_msg = "{\"error\": \"Invalid vector data\"}";
            struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                            (void*)error_msg, MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
            int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
            MHD_destroy_response(response);
            cJSON_Delete(json);
            free(vec.data);
            printf("post_handler_callback: Freeing con_data->data\n");
            free(con_data->data);
            printf("post_handler_callback: Freeing con_data\n");
            free(con_data);
            *con_cls = NULL;
            return ret == MHD_YES ? MHD_YES : MHD_NO;
        }
        // Store the vector data
        vec.data[i] = item->valuedouble;
    }
    printf("post_handler_callback: Extracted vector data, dimension: %zu\n", dimension);

    // Ensure the KDTree is initialized
    if (db->kdtree == NULL) {
        fprintf(stderr, "post_handler_callback: KDTree is NULL before inserting\n");
        if (db->size > 0 && db->vectors && db->vectors[0].data) {
            db->kdtree = kdtree_create(db->vectors[0].dimension);
        } else {
            db->kdtree = kdtree_create(dimension); // Default to the dimension of the current vector
        }
        if (db->kdtree == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to initialize KDTree\n");
            const char* error_msg = "{\"error\": \"Failed to initialize KDTree\"}";
            struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                            (void*)error_msg, MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
            int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
            cJSON_Delete(json);
            free(vec.data);
            printf("post_handler_callback: Freeing con_data->data\n");
            free(con_data->data);
            printf("post_handler_callback: Freeing con_data\n");
            free(con_data);
            *con_cls = NULL;
            return ret == MHD_YES ? MHD_YES : MHD_NO;
        }
    }

    // Insert the vector into the database
    printf("post_handler_callback: Inserting vector into database\n");
    size_t index = vector_db_insert(db, vec);
    if (index == (size_t)-1) {
        fprintf(stderr, "post_handler_callback: Failed to insert vector\n");
        const char* error_msg = "{\"error\": \"Failed to insert vector\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        cJSON_Delete(json);
        free(vec.data);
        printf("post_handler_callback: Freeing con_data->data\n");
        free(con_data->data);
        printf("post_handler_callback: Freeing con_data\n");
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    printf("post_handler_callback: Vector inserted at index %zu\n", index);

    // Clean up JSON data and connection data
    cJSON_Delete(json);
    printf("post_handler_callback: JSON deleted\n");
    printf("post_handler_callback: Freeing con_data->data\n");
    free(con_data->data);
    printf("post_handler_callback: Freeing con_data\n");
    free(con_data);
    *con_cls = NULL;

    // Prepare JSON response
    cJSON *response_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(response_json, "index", index);
    cJSON *vector_array = cJSON_CreateDoubleArray(vec.data, vec.dimension);
    cJSON_AddItemToObject(response_json, "vector", vector_array);
    char *response_str = cJSON_PrintUnformatted(response_json);
    cJSON_Delete(response_json);

    // Create and send the HTTP response
    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void*)response_str, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}
