#ifndef DELETE_HANDLER_H
#define DELETE_HANDLER_H

#include <microhttpd.h>
#include "vector_database.h"

struct DeleteHandlerData {
    VectorDatabase *db;
};

extern enum MHD_Result delete_handler(void *cls, struct MHD_Connection *connection,
                                      const char *url, const char *method,
                                      const char *version, const char *upload_data,
                                      size_t *upload_data_size, void **con_cls);

#endif /* DELETE_HANDLER_H */
