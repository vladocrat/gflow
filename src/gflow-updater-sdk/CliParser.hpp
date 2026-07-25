// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <cstdint>

#include <utils-cpp/pimpl.h>

enum class Command : uint8_t
{
    None,
    Update
};

class CliParser final
{
public:
    CliParser();
    ~CliParser();

    CliParser(const CliParser&)            = delete;
    CliParser& operator=(const CliParser&) = delete;
    CliParser(CliParser&&)                 = delete;
    CliParser& operator=(CliParser&&)      = delete;

    [[nodiscard]] int parse(int argc, char** argv);

    [[nodiscard]] Command selected() const;

private:
    DECLARE_PIMPL
};
