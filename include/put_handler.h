#ifndef PUT_HANDLER_H
#define PUT_HANDLER_H

#include <microhttpd.h>
#include "vector_database.h"

struct PutHandlerData {
    VectorDatabase *db;
};

extern enum MHD_Result put_handler(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls);

#endif /* PUT_HANDLER_H */
