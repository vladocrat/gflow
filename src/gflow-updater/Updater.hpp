// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>

struct Update
{
    std::filesystem::path from;
    std::filesystem::path to;
};

class Updater
{
public:
    static bool execute(const Update&) noexcept;
    static bool rollback(const Update&) noexcept;

private:
    static std::error_code cleanTemp() noexcept;
};
