// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include <CLI/CLI.hpp>

#include "CliParser.hpp"

int main(int argc, char** argv)
{
    CliParser parser;
    return parser.parse(argc, argv);
}
