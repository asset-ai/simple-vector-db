#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <microhttpd.h>

#include "vector_database.h"
#include "get_handler.h"
#include "post_handler.h"
#include "put_handler.h"
#include "delete_handler.h"
#include "compare_handler.h"

#define DEFAULT_PORT 8888
#define DB_FILENAME "vector_database.db"
#define DEFAULT_DIMENSION 3  // Example default dimension

/**
 * @brief Structure to hold connection-specific data.
 */
typedef struct {
    char *data;     /**< Data buffer */
    size_t data_size; /**< Size of the data buffer */
} ConnectionData;

/**
 * @brief Callback function to clean up request-specific data.
 * 
 * @param cls User-defined data.
 * @param connection Connection handle.
 * @param con_cls Connection-specific data.
 * @param toe Termination code.
 */
static void request_completed_callback(void* cls, struct MHD_Connection* connection,
                                       void** con_cls, enum MHD_RequestTerminationCode toe);

/**
 * @brief Function to handle GET requests.
 * 
 * @param cls User-defined data.
 * @param connection Connection handle.
 * @param url URL of the request.
 * @param method HTTP method.
 * @param version HTTP version.
 * @param upload_data Data being uploaded.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_get(void *cls, struct MHD_Connection *connection,
                               const char *url, const char *method,
                               const char *version, const char *upload_data,
                               size_t *upload_data_size, void **con_cls) {
    struct GetHandlerData* handler_data = (struct GetHandlerData*)cls;
    return get_handler(handler_data->db, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Function to handle POST requests.
 * 
 * @param cls User-defined data.
 * @param connection Connection handle.
 * @param url URL of the request.
 * @param method HTTP method.
 * @param version HTTP version.
 * @param upload_data Data being uploaded.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_post(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {
    struct PostHandlerData* handler_data = (struct PostHandlerData*)cls;
    return post_handler(handler_data->db, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Function to handle PUT requests.
 * 
 * @param cls User-defined data.
 * @param connection Connection handle.
 * @param url URL of the request.
 * @param method HTTP method.
 * @param version HTTP version.
 * @param upload_data Data being uploaded.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_put(void *cls, struct MHD_Connection *connection,
                               const char *url, const char *method,
                               const char *version, const char *upload_data,
                               size_t *upload_data_size, void **con_cls) {
    struct PutHandlerData* handler_data = (struct PutHandlerData*)cls;
    return put_handler(handler_data->db, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Function to handle DELETE requests.
 * 
 * @param cls User-defined data.
 * @param connection Connection handle.
 * @param url URL of the request.
 * @param method HTTP method.
 * @param version HTTP version.
 * @param upload_data Data being uploaded.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_delete(void *cls, struct MHD_Connection *connection,
                                  const char *url, const char *method,
                                  const char *version, const char *upload_data,
                                  size_t *upload_data_size, void **con_cls) {
    struct DeleteHandlerData* handler_data = (struct DeleteHandlerData*)cls;
    return delete_handler(handler_data->db, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Function to handle comparison requests.
 * 
 * @param cls User-defined data.
 * @param connection Connection handle.
 * @param url URL of the request.
 * @param method HTTP method.
 * @param version HTTP version.
 * @param upload_data Data being uploaded.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_compare(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls) {
    struct CompareHandlerData* handler_data = (struct CompareHandlerData*)cls;
    return compare_handler(handler_data->db, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Function to handle nearest neighbor requests.
 * 
 * @param cls User-defined data.
 * @param connection Connection handle.
 * @param url URL of the request.
 * @param method HTTP method.
 * @param version HTTP version.
 * @param upload_data Data being uploaded.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result ahc_nearest(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls) {
    struct CompareHandlerData* handler_data = (struct CompareHandlerData*)cls;
    return nearest_handler(handler_data->db, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

/**
 * @brief Main access handler function to route requests to appropriate handlers.
 * 
 * @param cls User-defined data.
 * @param connection Connection handle.
 * @param url URL of the request.
 * @param method HTTP method.
 * @param version HTTP version.
 * @param upload_data Data being uploaded.
 * @param upload_data_size Size of the upload data.
 * @param con_cls Connection-specific data.
 * @return MHD_Result indicating the success or failure of the operation.
 */
static enum MHD_Result access_handler(void *cls, struct MHD_Connection *connection,
                                      const char *url, const char *method,
                                      const char *version, const char *upload_data,
                                      size_t *upload_data_size, void **con_cls) {
    // Handle GET requests
    if (strcmp(method, "GET") == 0) {
        if (strcmp(url, "/vector") == 0) {
            return ahc_get(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
        } else if (strcmp(url, "/compare/cosine_similarity") == 0 || strcmp(url, "/compare/euclidean_distance") == 0 || strcmp(url, "/compare/dot_product") == 0) {
            return ahc_compare(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
        }
    }
    // Handle POST requests
    else if (strcmp(method, "POST") == 0) {
        if (strcmp(url, "/vector") == 0) {
            return ahc_post(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
        } else if (strcmp(url, "/nearest") == 0) {
            return ahc_nearest(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
        }
    }
    // Handle PUT requests
    else if (strcmp(method, "PUT") == 0 && strcmp(url, "/vector") == 0) {
        return ahc_put(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
    }
    // Handle DELETE requests
    else if (strcmp(method, "DELETE") == 0 && strcmp(url, "/vector") == 0) {
        return ahc_delete(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
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
 * @brief Callback function to clean up request-specific data.
 * 
 * @param cls User-defined data.
 * @param connection Connection handle.
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
 * @brief Main function to initialize and start the HTTP server.
 * 
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int Exit status.
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
            printf("), Median Point: %f\n", vec->median_point);
        } else {
            printf("Failed to read vector at index %zu\n", i);
        }
    }

    struct MHD_Daemon *daemon;

    // Pass the database through the handler data structure
    struct CompareHandlerData handler_data = { db };

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
