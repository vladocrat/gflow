// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>

#include <utils-cpp/pimpl.h>

class Storage;

class UpdateStorageService final
{
public:
    explicit UpdateStorageService(std::unique_ptr<Storage>&&);
    ~UpdateStorageService();

    UpdateStorageService() = delete;
    UpdateStorageService(const UpdateStorageService&) = delete;
    UpdateStorageService(UpdateStorageService&&) = delete;
    UpdateStorageService& operator=(const UpdateStorageService&) = delete;
    UpdateStorageService& operator=(UpdateStorageService&&) = delete;

    std::error_code backupCurrentVersion(const std::filesystem::path& applicationDir) const;
    std::error_code restoreBackup(const std::filesystem::path& applicationDir) const;
    std::error_code installUpdate(const std::filesystem::path& applicationDir) const;
    std::error_code clearStorage() const;

private:
    DECLARE_PIMPL
};
