// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>

class Updater
{
public:
    static bool update(const std::filesystem::path& from, const std::filesystem::path& to);
};

