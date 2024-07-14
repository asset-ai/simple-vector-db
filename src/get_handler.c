#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>
#include "cjson/cJSON.h"

#include "vector_database.h"

/**
 * @brief Callback function to handle GET requests.
 *
 * @param cls User-defined data, in this case, the database pointer.
 * @param connection MHD_Connection object representing the connection.
 * @param url URL of the request.
 * @param method HTTP method (should be "GET").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request (not used here).
 * @param upload_data_size Size of the upload data (not used here).
 * @param con_cls Connection-specific data (not used here).
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result get_handler_callback(void* cls, struct MHD_Connection* connection,
                                            const char* url, const char* method,
                                            const char* version,
                                            const char* upload_data,
                                            size_t* upload_data_size, void** con_cls);

/**
 * @brief Structure to hold data for the GET handler.
 */
struct GetHandlerData {
    VectorDatabase* db; /**< Pointer to the vector database */
};

/**
 * @brief Function to handle GET requests.
 *
 * @param cls User-defined data, in this case, the database pointer.
 * @param connection MHD_Connection object representing the connection.
 * @param url URL of the request.
 * @param method HTTP method (should be "GET").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request (not used here).
 * @param upload_data_size Size of the upload data (not used here).
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
enum MHD_Result get_handler(void* cls, struct MHD_Connection* connection,
                            const char* url, const char* method,
                            const char* version,
                            const char* upload_data,
                            size_t* upload_data_size, void** con_cls) {
    struct GetHandlerData* handler_data = (struct GetHandlerData*)cls;
    return get_handler_callback(handler_data->db, connection, url, method, version,
                                upload_data, upload_data_size, con_cls);
}

/**
 * @brief Callback function to handle GET request data.
 *
 * @param cls User-defined data, in this case, the database pointer.
 * @param connection MHD_Connection object representing the connection.
 * @param url URL of the request.
 * @param method HTTP method (should be "GET").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request (not used here).
 * @param upload_data_size Size of the upload data (not used here).
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result get_handler_callback(void* cls, struct MHD_Connection* connection,
                                            const char* url, const char* method,
                                            const char* version,
                                            const char* upload_data,
                                            size_t* upload_data_size, void** con_cls) {
    VectorDatabase* db = (VectorDatabase*)cls;
    if (!db) {
        fprintf(stderr, "Database pointer is NULL in handler callback\n");
        return MHD_NO;
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
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Convert the 'index' query parameter to a size_t value
    size_t index = atoi(index_str);
    if (index >= db->size) {
        // Respond with an error if the index is out of bounds
        const char* error_msg = "{\"error\": \"Index out of bounds\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Read the vector from the database
    Vector* vec = vector_db_read(db, index);
    if (!vec) {
        // Respond with an error if the vector is not found
        const char* error_msg = "{\"error\": \"Vector not found\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Prepare the JSON response
    cJSON* json_response = cJSON_CreateObject();
    if (!json_response) {
        // Respond with an error if JSON creation fails
        const char* error_msg = "{\"error\": \"Failed to create JSON object\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Convert the vector to a JSON array with precise float formatting
    cJSON* json_array = cJSON_CreateArray();
    if (!json_array) {
        cJSON_Delete(json_response);
        const char* error_msg = "{\"error\": \"Failed to create JSON array\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Add vector data to the JSON array
    for (size_t i = 0; i < vec->dimension; ++i) {
        cJSON_AddItemToArray(json_array, cJSON_CreateNumber(vec->data[i]));
    }

    // Add vector details to the JSON response
    cJSON_AddNumberToObject(json_response, "index", index);
    cJSON_AddItemToObject(json_response, "vector", json_array);
    cJSON_AddNumberToObject(json_response, "median_point", vec->median_point);

    // Convert JSON object to string
    char* response_str = cJSON_PrintUnformatted(json_response);
    cJSON_Delete(json_response);

    if (!response_str) {
        // Respond with an error if JSON string conversion fails
        const char* error_msg = "{\"error\": \"Failed to print JSON\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Create and send the HTTP response
    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void*)response_str, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}
