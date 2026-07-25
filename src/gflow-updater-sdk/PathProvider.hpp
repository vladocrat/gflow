// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>
#include <expected>

namespace paths
{

[[nodiscard]] std::expected<std::filesystem::path, std::error_code> getLocalAppData();
[[nodiscard]] std::expected<std::filesystem::path, std::error_code> getApplicationDataRootDir();

[[nodiscard]] std::expected<std::filesystem::path, std::error_code> createTempPatchDir();
[[nodiscard]] std::expected<std::filesystem::path, std::error_code> getTempPatchDir();

[[nodiscard]] std::expected<std::filesystem::path, std::error_code> createTempRollbackDir();
[[nodiscard]] std::expected<std::filesystem::path, std::error_code> getTempRollbackDir();

}
