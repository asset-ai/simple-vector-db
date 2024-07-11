#ifndef COMPARE_HANDLER_H
#define COMPARE_HANDLER_H

#include <microhttpd.h>
#include "vector_database.h"

struct CompareHandlerData {
    VectorDatabase* db;
};

enum MHD_Result compare_handler(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                size_t* upload_data_size, void** con_cls);

float cosine_similarity(Vector vec1, Vector vec2);

#endif /* COMPARE_HANDLER_H */
