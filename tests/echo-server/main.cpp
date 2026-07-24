// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include <spdlog/spdlog.h>

#include "EchoServer.hpp"

int main()
{
    const std::string address = "0.0.0.0:50051";
    gflow::demo::EchoServer server(address);

    if (server.port() == 0) {
        spdlog::error("echo-server: failed to start on {}", address);
        return 1;
    }

    spdlog::info("echo-server: listening on {}", address);
    server.wait();
    return 0;
}
