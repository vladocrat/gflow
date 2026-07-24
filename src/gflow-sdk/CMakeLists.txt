add_library(gflow-sdk STATIC
  GRPCClient.hpp GRPCClient.cpp
  ServerStream.hpp ServerStream.cpp
  ProtoModel.hpp ProtoModel.cpp
  Utils.hpp Utils.cpp
  LuaTypeGenerator.hpp LuaTypeGenerator.cpp
  Bridge.hpp Bridge.cpp
)

target_include_directories(gflow-sdk PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(gflow-sdk
  PUBLIC
  protobuf::libprotobuf
  gRPC::grpc++
  spdlog::spdlog
  sol2::sol2
  lua::lua
  utils-cpp
)
