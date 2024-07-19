// post_handler.h

#ifndef POST_HANDLER_H
#define POST_HANDLER_H

#include "vector_database.h"

/**
 * @brief Structure to hold data for the POST handler.
 */
typedef struct {
    VectorDatabase* db;
    size_t db_vector_size;
} PostHandlerData;

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
                             size_t* upload_data_size, void** con_cls);

#endif // POST_HANDLER_H
