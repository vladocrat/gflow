// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include "LocalStorage.hpp"

#include "PathProvider.hpp"

std::expected<std::filesystem::path, std::error_code> LocalStorage::acquireRollbackDir()
{
    return paths::getTempRollbackDir();
}

std::expected<std::filesystem::path, std::error_code> LocalStorage::acquirePatchDir()
{
    return paths::getTempPatchDir();
}
