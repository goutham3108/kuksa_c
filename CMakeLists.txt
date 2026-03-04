cmake_minimum_required(VERSION 3.16)
project(kuksa_c_client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(KUKSA_USE_PKG_CONFIG "Resolve gRPC/protobuf-c through pkg-config" ON)
set(KUKSA_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated" CACHE PATH "Directory containing generated protobuf-c files")

set(KUKSA_PROTO_SRCS
    "${KUKSA_GENERATED_DIR}/kuksa/val/v1/types.pb-c.c"
    "${KUKSA_GENERATED_DIR}/kuksa/val/v1/val.pb-c.c")

set(KUKSA_PROTO_HDRS
    "${KUKSA_GENERATED_DIR}/kuksa/val/v1/types.pb-c.h"
    "${KUKSA_GENERATED_DIR}/kuksa/val/v1/val.pb-c.h")

foreach(required_file IN LISTS KUKSA_PROTO_SRCS KUKSA_PROTO_HDRS)
  if(NOT EXISTS "${required_file}")
    message(FATAL_ERROR
      "Missing generated protobuf-c file: ${required_file}\n"
      "Run ./scripts/generate_proto.sh <path-to-kuksa-databroker> first.")
  endif()
endforeach()

if(KUKSA_USE_PKG_CONFIG)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GRPC REQUIRED grpc)

  # Distro differences:
  # - Debian/Ubuntu often ship libprotobuf-c.pc (module: libprotobuf-c)
  # - Other setups may use protobuf-c.pc (module: protobuf-c)
  pkg_check_modules(PROTOBUF_C QUIET protobuf-c)
  if(NOT PROTOBUF_C_FOUND)
    pkg_check_modules(PROTOBUF_C REQUIRED libprotobuf-c)
  endif()
endif()

add_executable(kuksa-c-cli
  src/main.c
  src/kuksa_client.c
  ${KUKSA_PROTO_SRCS}
)

target_include_directories(kuksa-c-cli PRIVATE
  ${KUKSA_GENERATED_DIR}
  src
  ${GRPC_INCLUDE_DIRS}
  ${PROTOBUF_C_INCLUDE_DIRS}
)

target_link_directories(kuksa-c-cli PRIVATE
  ${GRPC_LIBRARY_DIRS}
  ${PROTOBUF_C_LIBRARY_DIRS}
)

target_link_libraries(kuksa-c-cli PRIVATE
  ${GRPC_LIBRARIES}
  ${PROTOBUF_C_LIBRARIES}
)

target_compile_options(kuksa-c-cli PRIVATE
  ${GRPC_CFLAGS_OTHER}
  ${PROTOBUF_C_CFLAGS_OTHER}
)
