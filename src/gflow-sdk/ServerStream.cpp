#include "ServerStream.hpp"

#include <grpcpp/impl/rpc_method.h>
#include <grpcpp/impl/proto_utils.h>
#include <grpcpp/support/sync_stream.h>

namespace gflow
{

namespace
{

std::string methodPath(const google::protobuf::MethodDescriptor* method)
{
    const absl::string_view serviceName = method->service()->full_name();
    const absl::string_view methodName = method->name();

    std::string path;
    path += '/';
    path.append(serviceName.data(), serviceName.size());
    path += '/';
    path.append(methodName.data(), methodName.size());
    return path;
}

} // namespace

struct ServerStream::impl_t
{
    grpc::ClientContext context;
    std::unique_ptr<grpc::ClientReader<google::protobuf::Message>> reader;
    bool finished {false};
};

ServerStream::ServerStream(
    const std::shared_ptr<grpc::Channel>& channel, const google::protobuf::MethodDescriptor* method,
    const google::protobuf::Message& request, std::chrono::milliseconds timeout
)
{
    createImpl();
    impl().context.set_deadline(std::chrono::system_clock::now() + timeout);

    const std::string path = methodPath(method);
    const grpc::internal::RpcMethod rpcMethod(path.c_str(), grpc::internal::RpcMethod::SERVER_STREAMING);

    impl().reader.reset(grpc::internal::ClientReaderFactory<google::protobuf::Message>::Create(
        channel.get(), rpcMethod, &impl().context, request
    ));
}

ServerStream::~ServerStream()
{
    if (impl().reader && !impl().finished) {
        impl().context.TryCancel();
        impl().reader->Finish();
    }
}

bool ServerStream::read(google::protobuf::Message* message)
{
    return impl().reader->Read(message);
}

grpc::Status ServerStream::finish()
{
    impl().finished = true;
    return impl().reader->Finish();
}

void ServerStream::cancel()
{
    impl().context.TryCancel();
}

} // namespace gflow
