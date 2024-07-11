#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>

#include "../include/vector_database.h"

static enum MHD_Result delete_handler_callback(void* cls, struct MHD_Connection* connection,
                                              const char* url, const char* method,
                                              const char* version, const char* upload_data,
                                              size_t* upload_data_size, void** con_cls);

struct DeleteHandlerData {
    VectorDatabase* db;
};

enum MHD_Result delete_handler(void* cls, struct MHD_Connection* connection,
                               const char* url, const char* method,
                               const char* version, const char* upload_data,
                               size_t* upload_data_size, void** con_cls) {
    struct DeleteHandlerData* handler_data = (struct DeleteHandlerData*)cls;
    return delete_handler_callback(handler_data->db, connection, url, method, version,
                                   upload_data, upload_data_size, con_cls);
}

static enum MHD_Result delete_handler_callback(void* cls, struct MHD_Connection* connection,
                                              const char* url, const char* method,
                                              const char* version, const char* upload_data,
                                              size_t* upload_data_size, void** con_cls) {
    const char* index_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index");
    if (!index_str) {
        const char* error_msg = "Missing 'index' query parameter";
        struct MHD_Response* response = MHD_create_response_from_buffer(strlen(error_msg),
                                                                        (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    size_t index = atoi(index_str);
    vector_db_delete((VectorDatabase*)cls, index);
    struct MHD_Response* response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}
