#pragma once

#include <chrono>
#include <string>

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

class GRPCClient
{
public:
    explicit GRPCClient(const std::string& address, uint16_t port);
    virtual ~GRPCClient();

    GRPCClient(const GRPCClient&) = delete;
    GRPCClient& operator=(const GRPCClient&) = delete;
    GRPCClient(GRPCClient&&) = delete;
    GRPCClient& operator=(GRPCClient&&) = delete;

    grpc::Status unaryCall(
        const google::protobuf::MethodDescriptor* method, const google::protobuf::Message& request,
        google::protobuf::Message* response, std::chrono::milliseconds timeout = std::chrono::seconds(5)
    );

    std::unique_ptr<ServerStream> serverStreamingCall(
        const google::protobuf::MethodDescriptor* method, const google::protobuf::Message& request,
        std::chrono::milliseconds timeout = std::chrono::seconds(30)
    );

private:
    DECLARE_PIMPL
};

} // namespace gflow
