#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>

#include "../include/vector_database.h"
#include "../include/delete_handler.h"

/**
 * @brief Callback function to handle DELETE request data.
 * 
 * @param cls User-defined data, in this case, the vector database.
 * @param connection Pointer to MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "DELETE").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result delete_handler_callback(void* cls, struct MHD_Connection* connection,
                                               const char* url, const char* method,
                                               const char* version, const char* upload_data,
                                               size_t* upload_data_size, void** con_cls);

/**
 * @brief Function to handle DELETE requests.
 * 
 * @param cls User-defined data, in this case, the vector database.
 * @param connection Pointer to MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "DELETE").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
enum MHD_Result delete_handler(void* cls, struct MHD_Connection* connection,
                               const char* url, const char* method,
                               const char* version, const char* upload_data,
                               size_t* upload_data_size, void** con_cls) {
    struct DeleteHandlerData* handler_data = (struct DeleteHandlerData*)cls;
    return delete_handler_callback(handler_data->db, connection, url, method, version,
                                   upload_data, upload_data_size, con_cls);
}

/**
 * @brief Callback function to handle DELETE request data.
 * 
 * @param cls User-defined data, in this case, the vector database.
 * @param connection Pointer to MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "DELETE").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result delete_handler_callback(void* cls, struct MHD_Connection* connection,
                                               const char* url, const char* method,
                                               const char* version, const char* upload_data,
                                               size_t* upload_data_size, void** con_cls) {
    VectorDatabase* db = (VectorDatabase*)cls;

    // Retrieve the 'index' query parameter
    const char* index_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index");
    if (!index_str) {
        // Respond with an error if 'index' is missing
        const char* error_msg = "Missing 'index' query parameter";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Convert the 'index' query parameter to a size_t value
    size_t index = atoi(index_str);
    if (index >= db->size) {
        // Respond with an error if the index is out of bounds
        const char* error_msg = "Index out of bounds";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // Delete the vector from the database
    vector_db_delete(db, index);

    // Respond with an empty response to indicate success
    struct MHD_Response* response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}
