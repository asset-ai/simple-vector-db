#ifndef CONNECTION_DATA_H
#define CONNECTION_DATA_H
/**
 * @struct ConnectionData
 * @brief Structure to hold connection data.
 */
typedef struct ConnectionData {
    char *data;      ///< Pointer to data buffer
    size_t data_size; ///< Size of the data buffer
} ConnectionData;

#endif // CONNECTION_DATA_H
