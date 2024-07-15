#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <microhttpd.h>

#include "../include/vector_database.h"
#include "../include/get_handler.h"
#include "../include/post_handler.h"
#include "../include/put_handler.h"
#include "../include/delete_handler.h"
#include "../include/compare_handler.h"

#define DEFAULT_PORT 8888
#define DB_FILENAME "vector_database.db"
#define DEFAULT_DIMENSION 3  ///< Example default dimension

/**
 * @brief Structure to hold connection data.
 */
typedef struct {
    char *data;      ///< Pointer to data buffer
    size_t data_size; ///< Size of the data buffer
} ConnectionData;

/**
 * @brief Callback function called when a request is completed.
 *
 * @param cls User-defined data.
 * @param connection The connection object.
 * @param con_cls Connection-specific data.
 * @param toe Termination code.
 */
static void request_completed_callback(void* cls, struct MHD_Connection* connection,
                                       void** con_cls, enum MHD_RequestTerminationCode toe);

/**
 * @brief Handler function for GET requests.
 *
 * @param cls User-defined data.
 * @param connection The connection object.
 * @param url The URL of the request.
 * @param method The HTTP method.
 * @param version The HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_get(void *cls, struct MHD_Connection *connection,
                               const char *url, const char *method,
                               const char *version, const char *upload_data,
                               size_t *upload_data_size, void **con_cls) {
    return get_handler(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Handler function for POST requests.
 *
 * @param cls User-defined data.
 * @param connection The connection object.
 * @param url The URL of the request.
 * @param method The HTTP method.
 * @param version The HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_post(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {
    return post_handler(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Handler function for PUT requests.
 *
 * @param cls User-defined data.
 * @param connection The connection object.
 * @param url The URL of the request.
 * @param method The HTTP method.
 * @param version The HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_put(void *cls, struct MHD_Connection *connection,
                               const char *url, const char *method,
                               const char *version, const char *upload_data,
                               size_t *upload_data_size, void **con_cls) {
    return put_handler(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Handler function for DELETE requests.
 *
 * @param cls User-defined data.
 * @param connection The connection object.
 * @param url The URL of the request.
 * @param method The HTTP method.
 * @param version The HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_delete(void *cls, struct MHD_Connection *connection,
                                  const char *url, const char *method,
                                  const char *version, const char *upload_data,
                                  size_t *upload_data_size, void **con_cls) {
    return delete_handler(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Handler function for compare requests.
 *
 * @param cls User-defined data.
 * @param connection The connection object.
 * @param url The URL of the request.
 * @param method The HTTP method.
 * @param version The HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_compare(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls) {
    return compare_handler(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Handler function for nearest neighbor requests.
 *
 * @param cls User-defined data.
 * @param connection The connection object.
 * @param url The URL of the request.
 * @param method The HTTP method.
 * @param version The HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_nearest(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls) {
    return nearest_handler(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Main access handler function to route HTTP requests.
 *
 * @param cls User-defined data.
 * @param connection The connection object.
 * @param url The URL of the request.
 * @param method The HTTP method.
 * @param version The HTTP version.
 * @param upload_data Data being uploaded in the request.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result access_handler(void *cls, struct MHD_Connection *connection,
                                      const char *url, const char *method,
                                      const char *version, const char *upload_data,
                                      size_t *upload_data_size, void **con_cls) {
    PostHandlerData* handler_data = (PostHandlerData*)cls;
    VectorDatabase* db = handler_data->db;
    // Handle GET requests
    if (strcmp(method, "GET") == 0) {
        if (strcmp(url, "/vector") == 0) {
            return ahc_get(db, connection, url, method, version, upload_data, upload_data_size, con_cls);
        } else if (strcmp(url, "/compare/cosine_similarity") == 0 || strcmp(url, "/compare/euclidean_distance") == 0 || strcmp(url, "/compare/dot_product") == 0) {
            return ahc_compare(db, connection, url, method, version, upload_data, upload_data_size, con_cls);
        }
    }
    // Handle POST requests
    else if (strcmp(method, "POST") == 0) {
        if (strcmp(url, "/vector") == 0) {
            return ahc_post(handler_data, connection, url, method, version, upload_data, upload_data_size, con_cls);
        } else if (strcmp(url, "/nearest") == 0) {
            return ahc_nearest(db, connection, url, method, version, upload_data, upload_data_size, con_cls);
        }
    }
    // Handle PUT requests
    else if (strcmp(method, "PUT") == 0 && strcmp(url, "/vector") == 0) {
        return ahc_put(db, connection, url, method, version, upload_data, upload_data_size, con_cls);
    }
    // Handle DELETE requests
    else if (strcmp(method, "DELETE") == 0 && strcmp(url, "/vector") == 0) {
        return ahc_delete(db, connection, url, method, version, upload_data, upload_data_size, con_cls);
    }

    // Return 404 Not Found for unrecognized URLs
    const char *error_msg = "404 Not Found";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain");
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}

/**
 * @brief Callback function called when a request is completed.
 *
 * @param cls User-defined data.
 * @param connection The connection object.
 * @param con_cls Connection-specific data.
 * @param toe Termination code.
 */
static void request_completed_callback(void* cls, struct MHD_Connection* connection,
                                       void** con_cls, enum MHD_RequestTerminationCode toe) {
    ConnectionData *con_data = (ConnectionData*)*con_cls;
    if (con_data != NULL) {
        free(con_data->data);
        free(con_data);
        *con_cls = NULL;
    }
}

/**
 * @brief Main function to start the server and handle incoming requests.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int Status code.
 */
int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    size_t dimension = DEFAULT_DIMENSION;

    // Parse command-line arguments for port and dimension
    int opt;
    while ((opt = getopt(argc, argv, "p:d:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                dimension = (size_t)atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port] [-d dimension]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    VectorDatabase *db;

    // Load the database from file if it exists
    db = vector_db_load(DB_FILENAME, dimension);
    if (db == NULL) {
        // If loading failed, initialize a new database
        db = vector_db_init(0, dimension); // Passing 0 to initialize an empty database
        if (!db) {
            fprintf(stderr, "Failed to initialize vector database\n");
            return 1;
        }
    }

    PostHandlerData handler_data;
    handler_data.db = db;

    // Test initialization and reading of vectors
    for (size_t i = 0; i < db->size; i++) {
        Vector *vec = vector_db_read(db, i);
        if (vec) {
            printf("Read vector at index %zu: (", i);
            for (size_t j = 0; j < vec->dimension; j++) {
                printf("%f", vec->data[j]);
                if (j < vec->dimension - 1) {
                    printf(", ");
                }
            }
            printf(")\n");
        } else {
            printf("Failed to read vector at index %zu\n", i);
        }
    }

    struct MHD_Daemon *daemon;

    // Start the HTTP daemon
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port, NULL, NULL,
                              &access_handler, &handler_data,
                              MHD_OPTION_NOTIFY_COMPLETED, request_completed_callback, NULL,
                              MHD_OPTION_END);
    if (!daemon) {
        fprintf(stderr, "Failed to start server\n");
        vector_db_free(db);
        return 1;
    }

    printf("Server running on port %d\n", port);

    // Wait for user input to terminate the server
    getchar();

    // Save the database to file before shutting down
    vector_db_save(db, DB_FILENAME);

    // Stop the HTTP daemon and free the database
    MHD_stop_daemon(daemon);
    vector_db_free(db);

    return 0;
}
