// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include "UpdateStorageService.hpp"

#include <spdlog/spdlog.h>

#include "Storage.hpp"

struct UpdateStorageService::impl_t
{
    std::unique_ptr<Storage> storage;

    impl_t(std::unique_ptr<Storage>&& s)
        : storage {std::move(s)}
    {}

    ~impl_t() = default;

    impl_t() = delete;
    impl_t(const impl_t&) = delete;
    impl_t(impl_t&&) = delete;
    impl_t& operator=(const impl_t&) = delete;
    impl_t& operator=(impl_t&&) = delete;
};

UpdateStorageService::UpdateStorageService(std::unique_ptr<Storage>&& storage)
{
    createImpl(std::move(storage));
}

UpdateStorageService::~UpdateStorageService() = default;

std::error_code UpdateStorageService::backupCurrentVersion(const std::filesystem::path& applicationDir) const
{
    const auto rollbackDir = impl().storage->acquireRollbackDir();

    if (!rollbackDir) {
        spdlog::error("Failed to acquire temp dir, reason: {}", rollbackDir.error().message());
        return rollbackDir.error();
    }

    std::error_code errCode;
    std::filesystem::rename(applicationDir, rollbackDir.value(), errCode);

    if (errCode) {
        spdlog::error("Failed to move files to {}, reason: {}", rollbackDir.value().string(), errCode.message());
        return errCode;
    }

    return {};
}

std::error_code UpdateStorageService::restoreBackup(const std::filesystem::path& applicationDir) const
{
    const auto rollbackDir = impl().storage->acquireRollbackDir();

    if (!rollbackDir) {
        spdlog::error("Failed to acquire temp dir, reason: {}", rollbackDir.error().message());
        return rollbackDir.error();
    }

    std::error_code errCode;
    std::filesystem::remove_all(applicationDir, errCode);

    if (errCode) {
        spdlog::error("Failed to clean app dir {}, reason: {}", applicationDir.string(), errCode.message());
        return errCode;
    }

    std::filesystem::rename(rollbackDir.value(), applicationDir, errCode);

    if (errCode) {
        spdlog::error("Failed to move to app dir {}, reason: {}", applicationDir.string(), errCode.message());
        return errCode;
    }

    return {};
}

std::error_code UpdateStorageService::installUpdate(const std::filesystem::path& applicationDir) const
{
    const auto patchDir = impl().storage->acquirePatchDir();

    if (!patchDir) {
        spdlog::error("Failed to acquire patch dir, reason: {}", patchDir.error().message());
        return patchDir.error();
    }

    std::error_code errCode;
    std::filesystem::rename(patchDir.value(), applicationDir, errCode);

    if (errCode) {
        spdlog::error("Failed to move files to {}, reason: {}", applicationDir.string(), errCode.message());
        return errCode;
    }

    return {};
}

std::error_code UpdateStorageService::clearStorage() const
{
    std::error_code errCode;

    const auto patchDir = impl().storage->acquirePatchDir();

    if (!patchDir) {
        return patchDir.error();
    }

    std::filesystem::remove_all(patchDir.value(), errCode);

    if (errCode) {
        spdlog::warn("Failed to remove patch temp directory {}, reason: {}", patchDir.value().string(), errCode.message());
    }

    const auto rollbackDir = impl().storage->acquireRollbackDir();

    if (!rollbackDir) {
        return rollbackDir.error();
    }

    std::filesystem::remove_all(rollbackDir.value(), errCode);

    if (errCode) {
        spdlog::warn("Failed to remove temp directory {}, reason: {}", rollbackDir.value().string(), errCode.message());
    }

    return {};
}
