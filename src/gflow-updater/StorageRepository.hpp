// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>

#include <utils-cpp/pimpl.h>

class Storage;

class StorageRepository final
{
public:
    explicit StorageRepository(std::unique_ptr<Storage>&&);
    ~StorageRepository();

    StorageRepository() = delete;
    StorageRepository(const StorageRepository&) = delete;
    StorageRepository(StorageRepository&&) = delete;
    StorageRepository& operator=(const StorageRepository&) = delete;
    StorageRepository& operator=(StorageRepository&&) = delete;

    std::error_code backupCurrentVersion(const std::filesystem::path& applicationDir) const;
    std::error_code restoreBackup(const std::filesystem::path& applicationDir) const;
    std::error_code installUpdate(const std::filesystem::path& applicationDir) const;
    std::error_code clearStorage() const;

private:
    DECLARE_PIMPL
};
