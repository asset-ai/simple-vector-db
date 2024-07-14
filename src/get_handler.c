#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>
#include "cjson/cJSON.h"

#include "vector_database.h"

static enum MHD_Result get_handler_callback(void* cls, struct MHD_Connection* connection,
                                            const char* url, const char* method,
                                            const char* version,
                                            const char* upload_data,
                                            size_t* upload_data_size, void** con_cls);

struct GetHandlerData {
    VectorDatabase* db;
};

enum MHD_Result get_handler(void* cls, struct MHD_Connection* connection,
                            const char* url, const char* method,
                            const char* version,
                            const char* upload_data,
                            size_t* upload_data_size, void** con_cls) {
    struct GetHandlerData* handler_data = (struct GetHandlerData*)cls;
    return get_handler_callback(handler_data->db, connection, url, method, version,
                                upload_data, upload_data_size, con_cls);
}

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
    const char* index_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index");

    if (!index_str) {
        const char* error_msg = "{\"error\": \"Missing 'index' query parameter\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    size_t index = atoi(index_str);
    if (index >= db->size) {
        const char* error_msg = "{\"error\": \"Index out of bounds\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    Vector* vec = vector_db_read(db, index);
    if (!vec) {
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

    for (size_t i = 0; i < vec->dimension; ++i) {
        cJSON_AddItemToArray(json_array, cJSON_CreateNumber(vec->data[i]));
    }

    cJSON_AddNumberToObject(json_response, "index", index);
    cJSON_AddItemToObject(json_response, "vector", json_array);
    cJSON_AddNumberToObject(json_response, "median_point", vec->median_point);

    char* response_str = cJSON_PrintUnformatted(json_response);
    cJSON_Delete(json_response);

    if (!response_str) {
        const char* error_msg = "{\"error\": \"Failed to print JSON\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void*)response_str, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}
