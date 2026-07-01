#pragma once

#include <chrono>
#include <memory>
#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <grpcpp/grpcpp.h>

namespace gflow {

class GRPCClient
{
public:
    explicit GRPCClient(const std::string& address);

    grpc::Status unaryCall(const google::protobuf::MethodDescriptor* method,
                           const google::protobuf::Message& request, google::protobuf::Message* response,
                           std::chrono::milliseconds timeout = std::chrono::seconds(5));

private:
    std::shared_ptr<grpc::Channel> m_channel;
};

}
