find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

message(STATUS "grpc: using gRPC ${gRPC_VERSION} / protobuf ${protobuf_VERSION} from vcpkg")
