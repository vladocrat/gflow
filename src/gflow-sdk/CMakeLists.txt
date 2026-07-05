add_library(gflow-sdk STATIC
  GRPCClient.hpp GRPCClient.cpp
  ProtoModel.hpp ProtoModel.cpp
  Utils.hpp Utils.cpp
)

target_include_directories(gflow-sdk PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(gflow-sdk
  PUBLIC
  protobuf::libprotobuf
  gRPC::grpc++
  spdlog::spdlog
  utils-cpp
)
