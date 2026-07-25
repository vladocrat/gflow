// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include <CLI/CLI.hpp>

#include <spdlog/spdlog.h>

#include "CliParser.hpp"

int main(int argc, char** argv)
{
    spdlog::set_error_handler([](const std::string& msg) {
        std::fprintf(stderr, "Failed to write %s\n", msg.c_str());
    });

    CliParser parser;
    return parser.parse(argc, argv);
}
