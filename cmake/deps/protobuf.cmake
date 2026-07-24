if(NOT TARGET protobuf::libprotobuf)
  message(FATAL_ERROR
    "protobuf: not configured. Expected from vcpkg (see vcpkg.json / deps/grpc.cmake).")
endif()
