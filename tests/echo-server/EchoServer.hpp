// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <memory>
#include <string>

namespace grpc
{
class Server;
}

namespace gflow::demo
{

class EchoServiceImpl;

class EchoServer
{
public:
    explicit EchoServer(const std::string& address);
    ~EchoServer();

    EchoServer(const EchoServer&)            = delete;
    EchoServer& operator=(const EchoServer&) = delete;

    int port() const;
    void wait();

private:
    std::unique_ptr<EchoServiceImpl> m_service;
    std::unique_ptr<grpc::Server> m_server;
    int m_port = 0;
};

} // namespace gflow::demo
