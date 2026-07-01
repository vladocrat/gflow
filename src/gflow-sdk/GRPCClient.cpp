#include "GRPCClient.hpp"

#include <grpcpp/impl/rpc_method.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/impl/proto_utils.h>

namespace gflow {

GRPCClient::GRPCClient(const std::string& address)
    : m_channel(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()))
{
}

grpc::Status GRPCClient::unaryCall(const google::protobuf::MethodDescriptor* method,
                                   const google::protobuf::Message& request, google::protobuf::Message* response,
                                   std::chrono::milliseconds timeout)
{
    const absl::string_view serviceName = method->service()->full_name();
    const absl::string_view methodName = method->name();

    std::string path;
    path += '/';
    path.append(serviceName.data(), serviceName.size());
    path += '/';
    path.append(methodName.data(), methodName.size());

    grpc::internal::RpcMethod rpcMethod(path.c_str(), grpc::internal::RpcMethod::NORMAL_RPC);

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + timeout);

    return grpc::internal::BlockingUnaryCall<google::protobuf::Message, google::protobuf::Message>(
        m_channel.get(), rpcMethod, &context, request, response);
}

}
