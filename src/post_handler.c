#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>
#include "cjson/cJSON.h"

#include "../include/vector_database.h"
#include "../include/connection_data.h"
#include "../include/post_handler.h"


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
    PostHandlerData* handler_data = (PostHandlerData*)cls;
    printf("post_handler: Retrieved handler_data\n");
    return post_handler_callback(handler_data, connection, url, method, version,
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
    PostHandlerData* handler_data = (PostHandlerData*)cls;
    VectorDatabase* db = handler_data->db;
    size_t expected_vector_size = handler_data->db_vector_size;

    if (!db) {
        fprintf(stderr, "post_handler_callback: VectorDatabase is NULL\n");
        return MHD_NO;
    }

    if (*upload_data_size != 0) {
        char *new_data = (char *)realloc(con_data->data, con_data->data_size + *upload_data_size + 1);
        if (new_data == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to reallocate memory for data\n");
            free(con_data->data);
            free(con_data);
            *con_cls = NULL;
            return MHD_NO;
        }
        con_data->data = new_data;
        memcpy(con_data->data + con_data->data_size, upload_data, *upload_data_size);
        con_data->data_size += *upload_data_size;
        con_data->data[con_data->data_size] = '\0'; // Null-terminate the data
        *upload_data_size = 0;
        return MHD_YES;
    }

    if (con_data->data_size == 0) {
        fprintf(stderr, "post_handler_callback: Empty data received\n");
        const char* error_msg = "{\"error\": \"Empty data\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to create response\n");
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

    printf("post_handler_callback: Parsing JSON data\n");

    cJSON *json = cJSON_Parse(con_data->data);
    if (json == NULL) {
        fprintf(stderr, "post_handler_callback: Invalid JSON received\n");
        const char* error_msg = "{\"error\": \"Invalid JSON\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to create response\n");
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

    // Get the UUID from the JSON object
    cJSON *uuid_item = cJSON_GetObjectItem(json, "uuid");
    if (uuid_item == NULL || !cJSON_IsString(uuid_item)) {
        fprintf(stderr, "post_handler_callback: UUID is missing or invalid\n");
        const char* error_msg = "{\"error\": \"UUID is missing or invalid\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to create response\n");
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

    const char *uuid = uuid_item->valuestring;

    // Get the vector from the JSON object
    cJSON *vector_item = cJSON_GetObjectItem(json, "vector");
    if (vector_item == NULL || !cJSON_IsArray(vector_item)) {
        fprintf(stderr, "post_handler_callback: Vector is missing or invalid\n");
        const char* error_msg = "{\"error\": \"Vector is missing or invalid\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to create response\n");
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

    size_t dimension = cJSON_GetArraySize(vector_item);
    printf("post_handler_callback: Vector dimension: %zu\n", dimension);

    if (dimension != expected_vector_size) {
        fprintf(stderr, "post_handler_callback: Vector size mismatch. Expected %zu, got %zu\n", expected_vector_size, dimension);
        const char* error_msg = "{\"error\": \"Vector size mismatch\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to create response\n");
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
    strncpy(vec.uuid, uuid, sizeof(vec.uuid) - 1);
    vec.uuid[sizeof(vec.uuid) - 1] = '\0'; // Ensure null-termination
    vec.dimension = dimension;
    vec.data = (double*)malloc(dimension * sizeof(double));
    if (!vec.data) {
        fprintf(stderr, "post_handler_callback: Memory allocation for vector data failed\n");
        const char* error_msg = "{\"error\": \"Memory allocation failed\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to create response\n");
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
        cJSON *item = cJSON_GetArrayItem(vector_item, i);
        if (!cJSON_IsNumber(item)) {
            fprintf(stderr, "post_handler_callback: Invalid vector data at index %zu\n", i);
            const char* error_msg = "{\"error\": \"Invalid vector data\"}";
            struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
            if (response == NULL) {
                fprintf(stderr, "post_handler_callback: Failed to create response\n");
                cJSON_Delete(json);
                free(vec.data);
                free(con_data->data);
                free(con_data);
                *con_cls = NULL;
                return MHD_NO;
            }
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
    printf("post_handler_callback: Extracted vector data, dimension: %zu\n", dimension);

    if (db->kdtree == NULL) {
        fprintf(stderr, "post_handler_callback: KDTree is NULL before inserting\n");
        if (db->size > 0 && db->vectors && db->vectors[0].data) {
            db->kdtree = kdtree_create(db->vectors[0].dimension);
        } else {
            db->kdtree = kdtree_create(dimension);
        }
        if (db->kdtree == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to initialize KDTree\n");
            const char* error_msg = "{\"error\": \"Failed to initialize KDTree\"}";
            struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
            if (response == NULL) {
                fprintf(stderr, "post_handler_callback: Failed to create response\n");
                cJSON_Delete(json);
                free(vec.data);
                free(con_data->data);
                free(con_data);
                *con_cls = NULL;
                return MHD_NO;
            }
            MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
            int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
            MHD_destroy_response(response);
            cJSON_Delete(json);
            free(vec.data);
            free(con_data->data);
            free(con_data);
            *con_cls = NULL;
            return ret == MHD_YES ? MHD_YES : MHD_NO;
        }
    }

    printf("post_handler_callback: Inserting vector into database\n");
    size_t index = vector_db_insert(db, vec);
    if (index == (size_t)-1) {
        fprintf(stderr, "post_handler_callback: Failed to insert vector\n");
        const char* error_msg = "{\"error\": \"Failed to insert vector\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            fprintf(stderr, "post_handler_callback: Failed to create response\n");
            cJSON_Delete(json);
            free(vec.data);
            free(con_data->data);
            free(con_data);
            *con_cls = NULL;
            return MHD_NO;
        }
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        cJSON_Delete(json);
        free(vec.data);
        free(con_data->data);
        free(con_data);
        *con_cls = NULL;
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    printf("post_handler_callback: Vector inserted at index %zu\n", index);

    if (json) {
        printf("post_handler_callback: Deleting JSON\n");
        cJSON_Delete(json);
    }
    if (con_data && con_data->data) {
        printf("post_handler_callback: Freeing con_data->data\n");
        free(con_data->data);
    }
    if (con_data) {
        printf("post_handler_callback: Freeing con_data\n");
        free(con_data);
    }
    *con_cls = NULL;

    cJSON *response_json = cJSON_CreateObject();
    cJSON_AddStringToObject(response_json, "uuid", vec.uuid);
    cJSON_AddNumberToObject(response_json, "index", index);
    cJSON *vector_array = cJSON_CreateDoubleArray(vec.data, vec.dimension);
    cJSON_AddItemToObject(response_json, "vector", vector_array);
    char *response_str = cJSON_PrintUnformatted(response_json);
    cJSON_Delete(response_json);

    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void*)response_str, MHD_RESPMEM_MUST_FREE);
    if (response == NULL) {
        fprintf(stderr, "post_handler_callback: Failed to create response\n");
        return MHD_NO;
    }
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}
