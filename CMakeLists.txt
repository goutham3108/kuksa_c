cmake_minimum_required(VERSION 3.16)
project(kuksa_c_client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc)
pkg_check_modules(PROTOBUF_C REQUIRED protobuf-c)

add_executable(kuksa-c-cli
    src/main.c
    src/kuksa_client.c
)

target_include_directories(kuksa-c-cli PRIVATE
    ${GRPC_INCLUDE_DIRS}
    ${PROTOBUF_C_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
    src
)

target_link_directories(kuksa-c-cli PRIVATE ${GRPC_LIBRARY_DIRS} ${PROTOBUF_C_LIBRARY_DIRS})

target_link_libraries(kuksa-c-cli PRIVATE ${GRPC_LIBRARIES} ${PROTOBUF_C_LIBRARIES})

target_compile_options(kuksa-c-cli PRIVATE ${GRPC_CFLAGS_OTHER} ${PROTOBUF_C_CFLAGS_OTHER})
