#pragma once

#include <chrono>
#include <memory>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <grpcpp/grpcpp.h>

#include <utils-cpp/pimpl.h>

namespace gflow
{

class ServerStream final
{
public:
    ServerStream(
        const std::shared_ptr<grpc::Channel>& channel, const google::protobuf::MethodDescriptor* method,
        const google::protobuf::Message& request, std::chrono::milliseconds timeout
    );
    ~ServerStream();

    ServerStream(const ServerStream&) = delete;
    ServerStream& operator=(const ServerStream&) = delete;
    ServerStream(ServerStream&&) = delete;
    ServerStream& operator=(ServerStream&&) = delete;

    bool read(google::protobuf::Message* message);
    grpc::Status finish();
    void cancel();

private:
    DECLARE_PIMPL
};

} // namespace gflow
