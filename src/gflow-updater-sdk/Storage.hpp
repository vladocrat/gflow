// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>
#include <expected>

class Storage
{
public:
    Storage() noexcept = default;
    virtual ~Storage() noexcept = default;

    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) = delete;

    [[nodiscard]] virtual std::expected<std::filesystem::path, std::error_code> acquireRollbackDir() = 0;
    [[nodiscard]] virtual std::expected<std::filesystem::path, std::error_code> acquirePatchDir() = 0;
};
