#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>
#include "cjson/cJSON.h"

#include "../include/vector_database.h"
#include "../include/connection_data.h"
#include "../include/put_handler.h"

/**
 * @struct PostHandlerData
 * @brief Structure to hold data for the POST handler
 */
typedef struct {
    VectorDatabase* db;
    size_t db_vector_size;
} PostHandlerData;

/**
 * @brief Callback function to handle the PUT request data.
 * 
 * @param cls User-defined data, in this case, the handler data.
 * @param connection MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "PUT").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result put_handler_callback(void* cls, struct MHD_Connection* connection,
                                           const char* url, const char* method,
                                           const char* version, const char* upload_data,
                                           size_t* upload_data_size, void** con_cls);

/**
 * @brief Function to handle PUT requests.
 * 
 * @param cls User-defined data, in this case, the handler data.
 * @param connection MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "PUT").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
enum MHD_Result put_handler(void* cls, struct MHD_Connection* connection,
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
    return put_handler_callback(handler_data, connection, url, method, version,
                                upload_data, upload_data_size, con_cls);
}

/**
 * @brief Callback function to handle PUT request data.
 * 
 * @param cls User-defined data, in this case, the handler data.
 * @param connection MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "PUT").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result put_handler_callback(void* cls, struct MHD_Connection* connection,
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
        char *new_data = (char *)realloc(con_data->data, con_data->data_size + *upload_data_size + 1);
        if (new_data == NULL) {
            free(con_data->data);
            free(con_data);
            *con_cls = NULL;
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
        const char* error_msg = "{\"error\": \"Empty data\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            free(con_data->data);
            free(con_data);
            *con_cls = NULL;
            return MHD_NO;
        }
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        free(con_data->data);
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Retrieve the 'index' query parameter
    const char* index_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index");
    if (!index_str) {
        // Respond with an error if 'index' is missing
        const char* error_msg = "{\"error\": \"Missing 'index' query parameter\"}";
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

    // Convert the 'index' query parameter to a size_t value
    size_t index = atoi(index_str);

    // Check if the index is out of bounds
    if (index >= db->size) {
        const char* error_msg = "{\"error\": \"Index out of bounds\"}";
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

    printf("put_handler_callback: Parsing JSON data\n");

    // Parse the JSON data from the request
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

    printf("put_handler_callback: JSON parsed successfully\n");

    // Get the dimension of the vector from the JSON array size
    size_t dimension = cJSON_GetArraySize(json);
    printf("put_handler_callback: Vector dimension: %zu\n", dimension);

    // Check if the vector size matches the expected size
    if (dimension != expected_vector_size) {
        const char* error_msg = "{\"error\": \"Vector size mismatch\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
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

    // Extract the vector data from the JSON array
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
        // Store the vector data
        vec.data[i] = item->valuedouble;
    }
    printf("put_handler_callback: Extracted vector data, dimension: %zu\n", dimension);

    // Update the vector in the database
    vector_db_update(db, index, vec);
    
    // Clean up JSON data and connection data
    cJSON_Delete(json);
    free(con_data->data);
    free(con_data);
    *con_cls = NULL;

    // Respond with an empty response to indicate success
    struct MHD_Response* response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}
