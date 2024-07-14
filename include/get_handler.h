#ifndef GET_HANDLER_H
#define GET_HANDLER_H

#include <microhttpd.h>
#include "vector_database.h"

/**
 * @struct GetHandlerData
 * @brief Structure to hold data for the GET handler.
 */
struct GetHandlerData {
    VectorDatabase *db; /**< Pointer to the VectorDatabase structure */
};

/**
 * @brief Handles GET requests.
 * 
 * @param cls User-defined data, in this case, the database.
 * @param connection MHD_Connection object.
 * @param url URL of the request.
 * @param method HTTP method (should be "GET").
 * @param version HTTP version.
 * @param upload_data Data being uploaded in the request (should be empty for GET requests).
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
extern enum MHD_Result get_handler(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls);

#endif // GET_HANDLER_H
