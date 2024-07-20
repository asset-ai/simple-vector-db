#ifndef PUT_HANDLER_H
#define PUT_HANDLER_H

#include <microhttpd.h>

#include "vector_database.h"

/**
 * @struct PutHandlerData
 * @brief Structure to hold data for the PUT handler
 */
typedef struct {
    VectorDatabase* db;
    size_t db_vector_size;
} PutHandlerData;

/**
 * @brief Handles PUT requests.
 * 
 * @param cls User-defined data, in this case, the database.
 * @param connection MHD_Connection object representing the connection.
 * @param url URL of the request.
 * @param method HTTP method (should be "PUT").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
extern enum MHD_Result put_handler(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls);

#endif // PUT_HANDLER_H
