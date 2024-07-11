#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
