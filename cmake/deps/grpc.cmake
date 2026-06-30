set(GFLOW_GRPC_VERSION v1.73.1)

set(GFLOW_THIRD_PARTY "${CMAKE_SOURCE_DIR}/third_party" CACHE PATH "Prebuilt-dependency prefix")

list(APPEND CMAKE_PREFIX_PATH "${GFLOW_THIRD_PARTY}")

find_package(protobuf CONFIG QUIET)
find_package(gRPC CONFIG QUIET)

if(gRPC_FOUND)
  if(TARGET gRPC::grpc_cpp_plugin)
    set(gRPC_CPP_PLUGIN_EXECUTABLE "$<TARGET_FILE:gRPC::grpc_cpp_plugin>" CACHE STRING "" FORCE)
  endif()
  if(TARGET protobuf::protoc)
    set(Protobuf_PROTOC_EXECUTABLE "$<TARGET_FILE:protobuf::protoc>" CACHE STRING "" FORCE)
  endif()
  message(STATUS "grpc: using prebuilt ${GFLOW_GRPC_VERSION} from ${GFLOW_THIRD_PARTY}")
  return()
endif()
message(STATUS "grpc: prebuilt not found in ${GFLOW_THIRD_PARTY}; targets needing gRPC will be skipped.")

