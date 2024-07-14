# Simple Vector DB

Simple Vector DB is a lightweight, efficient, and easy-to-use vector database designed to store, retrieve, and manage high-dimensional vectors. It supports operations such as insertion, update, deletion, and comparison of vectors using cosine similarity, Euclidean distance, and dot product. Additionally, it allows for finding the nearest vector based on KD-tree median points.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
  - [Dependencies](#dependencies)
    - [macOS](#macos)
    - [Linux](#linux)
    - [Windows](#windows)
  - [Finding Library Paths](#finding-library-paths)
  - [Updating the Makefile](#updating-the-makefile)
- [Usage](#usage)
  - [Starting the Server](#starting-the-server)
    - [Fill Database with Dummy vector](#fill-database-with-dummy-vector)
  - [API Endpoints](#api-endpoints)
    - [Insert a Vector](#insert-a-vector)
    - [Retrieve a Vector](#retrieve-a-vector)
    - [Update a Vector](#update-a-vector)
    - [Delete a Vector](#delete-a-vector)
    - [Compare Vectors](#compare-vectors)
    - [Find Nearest Vector](#find-nearest-vector)
- [Build and Run](#build-and-run)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Efficient Vector Storage**: Stores high-dimensional vectors with dynamic allocation.
- **Vector Operations**: Supports insertion, retrieval, update, and deletion of vectors.
- **Comparison Metrics**: Compare vectors using cosine similarity, Euclidean distance, and dot product.
- **Nearest Vector Search**: Find the nearest vector based on KD-tree median points.
- **RESTful API**: Simple and intuitive API endpoints for easy integration.
- **Persistent Storage**: Save and load vector databases from disk.

## Installation

### Dependencies

Simple Vector DB depends on `libmicrohttpd` and `cJSON`. Here are the instructions to install these dependencies on different operating systems.

#### macOS

1. **libmicrohttpd**:
   ```sh
   brew install libmicrohttpd
   ```
2. **cJSON**:
   ```sh
   brew install cjson
   ```

#### Linux

For Debian-based distributions (e.g., Ubuntu):

1. **libmicrohttpd**:
   ```sh
   sudo apt-get update
   sudo apt-get install libmicrohttpd-dev
   ```
2. **cJSON**:
   ```sh
   sudo apt-get install libcjson-dev
   ```

For Red Hat-based distributions (e.g., CentOS, Fedora):

1. **libmicrohttpd**:
   ```sh
   sudo dnf install libmicrohttpd-devel
   ```
2. **cJSON**:
   ```sh
   sudo dnf install cjson-devel
   ```

#### Windows

For Windows, you can use a package manager like vcpkg to install the dependencies.

1. **Install vcpkg**:
   ```sh
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh
   ```

2. **libmicrohttpd**:
   ```sh
   ./vcpkg install libmicrohttpd
   ```

3. **cJSON**:
   ```sh
   ./vcpkg install cjson
   ```

### Finding Library Paths

After installing the libraries, you need to find the paths to `libmicrohttpd.h` and `cjson/cjson.h`.

1. **macOS and Linux**:
   - Typically, headers are located in `/usr/include`, `/usr/local/include`, or the installation prefix of the package manager (e.g., `/opt/homebrew/include` for Homebrew on macOS).

   - Libraries are usually in `/usr/lib`, `/usr/local/lib`, or the package manager prefix (e.g., `/opt/homebrew/lib`).

   - Use the `find` command to locate the header files:
     ```sh
     find /usr -name "libmicrohttpd.h"
     find /usr -name "cjson.h"
     ```

2. **Windows**:
   - vcpkg installs libraries in the `vcpkg/installed` directory. You can find headers and libraries in `vcpkg/installed/x64-windows/include` and `vcpkg/installed/x64-windows/lib`.

### Updating the Makefile

Update your Makefile to include the correct paths for the headers and libraries. Below are the two lines that need to be updated:

```makefile
CFLAGS = -Wall -I/opt/homebrew/include -I./include
LDFLAGS = -L/opt/homebrew/lib -lmicrohttpd -lcjson
```

Replace `/opt/homebrew/include` and `/opt/homebrew/lib` with the appropriate paths for your system.

## Usage

### Starting the Server

You can start the server on the default port (8888) or specify a custom port using the `-p` or `--port` flag.

```sh
# Start the server on the default port 8888
./executable/vector_db_server

# Start the server on a custom port (e.g., 8080)
./executable/vector_db_server -p 8080
```

### Fill Database with Dummy vector
You can fill the database with different vectors of different dimensions. Randomly generated.
```sh
# Change execution of the file
chmod +x ./test/add_vectors.sh
./test/add_vectors.sh
```

### API Endpoints

#### Insert a Vector

- **Endpoint**: `/vector`
- **Method**: `POST`
- **Request Body**: JSON array of float64 values.

```sh
curl -X POST -H "Content-Type: application/json" -d '[1.0, 2.0, 3.0, 4.0]' http://localhost:8888/vector
```

#### Retrieve a Vector

- **Endpoint**: `/vector`
- **Method**: `GET`
- **Query Parameter**: `index` (the index of the vector to retrieve).

```sh
curl "http://localhost:8888/vector?index=0"
```

#### Update a Vector

- **Endpoint**: `/vector`
- **Method**: `PUT`
- **Query Parameter**: `index` (the index of the vector to update).
- **Request Body**: JSON array of float64 values.

```sh
curl -X PUT -H "Content-Type: application/json" -d '[1.5, 2.5, 3.5, 4.5]' "http://localhost:8888/vector?index=0"
```

#### Delete a Vector

- **Endpoint**: `/vector`
- **Method**: `DELETE`
- **Query Parameter**: `index` (the index of the vector to delete).

```sh
curl -X DELETE "http://localhost:8888/vector?index=0"
```

#### Compare Vectors

- **Endpoint**: `/compare/cosine_similarity`
- **Method**: `GET`
- **Query Parameters**: `index1` and `index2` (the indices of the vectors to compare).

```sh
curl "http://localhost:8888/compare/cosine_similarity?index1=0&index2=1"
```

- **Endpoint**: `/compare/euclidean_distance`
- **Method**: `GET`
- **Query Parameters**: `index1` and `index2` (the indices of the vectors to compare).

```sh
curl "http://localhost:8888/compare/euclidean_distance?index1=0&index2=1"
```

- **Endpoint**: `/compare/dot_product`
- **Method**: `GET`
- **Query Parameters**: `index1` and `index2` (the indices of the vectors to compare).

```sh
curl "http://localhost:8888/compare/dot_product?index1=0&index2=1"
```

#### Find Nearest Vector

- **Endpoint**: `/nearest`
- **Method**: `POST`
- **Content-Type**: `application/json`
- **Request Body**: JSON array representing the input vector.
- **Optional query parameter**: `number=(int)` The number of nearest vectors to return - default is 1.


```sh
curl -X POST -H "Content-Type: application/json" -d '[7,3.00003,6.32,4.5,8,5,1.842,4.929066,7.94764,6.16051,6.946,4.71,4.3,1.704,2.321,5.9,6.74227,7.365,5.31,4.1705]' "http://localhost:8888/nearest"
```

**Response**:

```json
{
  "index": 2,
  "vector": [1.0, 2.0, 3.0, 4.08993, 5.937, 6.389, 1.39],
  "median_point": 3.0
}
```

This response indicates that the nearest vector is at index 2, and it includes the vector and its median point.

## Build and Run

To build and run Simple Vector DB, execute the following commands:

```sh
# Build the project
make

# Start the server
./executable/vector_db_server
```

To specify a custom port:

```sh
./executable/vector_db_server -p 8080
```

## Contributing

We welcome contributions to Simple Vector DB! Please fork the repository, create a new branch for your feature or bugfix, and submit a pull request.

1. Fork the repository.
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some new feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request.

## License

This project is licensed under the MIT

 License - see the [LICENSE](LICENSE) file for details.