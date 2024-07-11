#ifndef GET_HANDLER_H
#define GET_HANDLER_H

#include <microhttpd.h>
#include "vector_database.h"

struct GetHandlerData {
    VectorDatabase *db;
};

extern enum MHD_Result get_handler(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls);

#endif /* GET_HANDLER_H */
