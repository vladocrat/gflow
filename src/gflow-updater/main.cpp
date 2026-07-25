// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include <spdlog/spdlog.h>

#include <print>

#include "CliParser.hpp"

int main(int argc, char** argv)
{
    spdlog::set_error_handler([](const std::string& msg) {
        std::println(stderr, "Failed to write {}", msg);
    });

    CliParser parser;
    return parser.parse(argc, argv);
}
