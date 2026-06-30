if(NOT TARGET protobuf::libprotobuf)
  message(STATUS
    "protobuf: NOT configured (expected from the prebuilt gRPC in third_party; see grpc.cmake).")
endif()
