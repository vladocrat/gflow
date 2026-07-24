// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include "GRPCClient.hpp"

#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/impl/proto_utils.h>
#include <grpcpp/impl/rpc_method.h>

#include "ServerStream.hpp"

namespace gflow
{

namespace
{

std::string methodPath(const google::protobuf::MethodDescriptor* method)
{
    const absl::string_view serviceName = method->service()->full_name();
    const absl::string_view methodName  = method->name();

    std::string path;
    path += '/';
    path.append(serviceName.data(), serviceName.size());
    path += '/';
    path.append(methodName.data(), methodName.size());
    return path;
}

} // namespace

struct GRPCClient::impl_t
{
    std::shared_ptr<grpc::Channel> channel;

    impl_t(std::shared_ptr<grpc::Channel>&& ch)
    : channel {std::move(ch)}
    {}
};

GRPCClient::GRPCClient(const std::string& address, uint16_t port)
{
    const auto completeAddress = address + ":" + std::to_string(port);
    createImpl(grpc::CreateChannel(completeAddress, grpc::InsecureChannelCredentials()));
}

GRPCClient::~GRPCClient() = default;

grpc::Status GRPCClient::unaryCall(
    const google::protobuf::MethodDescriptor* method, const google::protobuf::Message& request,
    google::protobuf::Message* response, std::chrono::milliseconds timeout
)
{
    const std::string path = methodPath(method);
    const grpc::internal::RpcMethod rpcMethod(path.c_str(), grpc::internal::RpcMethod::NORMAL_RPC);

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + timeout);

    return grpc::internal::BlockingUnaryCall<google::protobuf::Message, google::protobuf::Message>(
        impl().channel.get(),
        rpcMethod,
        &context,
        request,
        response
    );
}

std::unique_ptr<ServerStream> GRPCClient::serverStreamingCall(
    const google::protobuf::MethodDescriptor* method, const google::protobuf::Message& request,
    std::chrono::milliseconds timeout
)
{
    return std::make_unique<ServerStream>(impl().channel, method, request, timeout);
}

} // namespace gflow
