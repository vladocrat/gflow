#include "EchoServer.hpp"

#include <grpcpp/grpcpp.h>

#include "echo.grpc.pb.h"

namespace gflow::demo {

class EchoServiceImpl final : public EchoService::Service
{
public:
    grpc::Status Echo(grpc::ServerContext* /*context*/, const EchoRequest* request, EchoResponse* response) override
    {
        response->set_message(request->message());
        response->set_length(static_cast<int32_t>(request->message().size()));
        response->set_color(request->color());
        return grpc::Status::OK;
    }

    grpc::Status Subscribe(grpc::ServerContext* /*context*/, const TickRequest* /*request*/,
                           grpc::ServerWriter<Tick>* /*writer*/) override
    {
        return grpc::Status::OK;
    }
};

EchoServer::EchoServer(const std::string& address)
    : m_service(std::make_unique<EchoServiceImpl>())
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials(), &m_port);
    builder.RegisterService(m_service.get());
    m_server = builder.BuildAndStart();
}

EchoServer::~EchoServer()
{
    if (m_server) {
        m_server->Shutdown();
    }
}

int EchoServer::port() const
{
    return m_port;
}

void EchoServer::wait()
{
    if (m_server) {
        m_server->Wait();
    }
}

}
