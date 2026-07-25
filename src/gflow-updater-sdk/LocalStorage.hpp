// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include "Storage.hpp"

class LocalStorage final : public Storage
{
public:
    [[nodiscard]] std::expected<std::filesystem::path, std::error_code> acquireRollbackDir() override;
    [[nodiscard]] std::expected<std::filesystem::path, std::error_code> acquirePatchDir() override;
};
