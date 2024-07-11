#ifndef POST_HANDLER_H
#define POST_HANDLER_H

#include <microhttpd.h>
#include "vector_database.h"

struct PostHandlerData {
    VectorDatabase *db;
};

extern enum MHD_Result post_handler(void *cls, struct MHD_Connection *connection,
                                    const char *url, const char *method,
                                    const char *version, const char *upload_data,
                                    size_t *upload_data_size, void **con_cls);

#endif /* POST_HANDLER_H */
