// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>
#include <expected>

namespace paths
{

[[nodiscard]] std::expected<std::filesystem::path, std::error_code> getLocalAppData() noexcept;
[[nodiscard]] std::expected<std::filesystem::path, std::error_code> getApplicationDataRootDir() noexcept;

[[nodiscard]] std::expected<std::filesystem::path, std::error_code> createTempPatchDir() noexcept;
[[nodiscard]] std::expected<std::filesystem::path, std::error_code> getTempPatchDir() noexcept;

[[nodiscard]] std::expected<std::filesystem::path, std::error_code> createTempRollbackDir() noexcept;
[[nodiscard]] std::expected<std::filesystem::path, std::error_code> getTempRollbackDir() noexcept;

}
