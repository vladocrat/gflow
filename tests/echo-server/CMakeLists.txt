add_library(echo-service STATIC
  EchoServer.hpp EchoServer.cpp
)

protobuf_generate(
  TARGET echo-service
  LANGUAGE cpp
  IMPORT_DIRS "${PROTO_DIR}"
  PROTOS "${PROTO_DIR}/echo.proto"
  PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

protobuf_generate(
  TARGET echo-service
  LANGUAGE grpc
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
  PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
  IMPORT_DIRS "${PROTO_DIR}"
  PROTOS "${PROTO_DIR}/echo.proto"
  PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

target_include_directories(echo-service PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(echo-service PUBLIC
  gRPC::grpc++
  protobuf::libprotobuf)

add_executable(echo-server main.cpp)

target_link_libraries(echo-server PRIVATE
  echo-service
  spdlog::spdlog)
