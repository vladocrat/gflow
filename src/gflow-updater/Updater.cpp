// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include "Updater.hpp"

#include <spdlog/spdlog.h>

#include "StorageRepository.hpp"

struct Updater::impl_t
{
    const StorageRepository& repository;

    impl_t(const StorageRepository& repo)
        : repository { repo }
    {

    }
};

Updater::Updater(const StorageRepository& repository) noexcept
{
    createImpl(repository);
}

Updater::~Updater() noexcept = default;

bool Updater::execute(const Update& update) const noexcept
{
    try {
        if (const auto err = impl().repository.backupCurrentVersion(update.to)) {
            spdlog::error("Failed to backup version: {}", err.message());
            return false;
        }

        if (const auto err = impl().repository.installUpdate(update.to)) {
            spdlog::error("Failed to install update: {}", err.message());
            return false;
        }
    } catch (const std::exception& e) {
        spdlog::error("Update failed, reason: {}", e.what());
        return false;
    } catch (...) {
        spdlog::error("Unknown exception");
        return false;
    }

    return true;
}

bool Updater::rollback(const Update& update) const noexcept
{
    try {
        if (const auto err = impl().repository.restoreBackup(update.to)) {
            spdlog::error("Failed to rollback version: {}", err.message());
            return false;
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to rollback update, reason: {}", e.what());
        return false;
    } catch (...) {
        spdlog::error("Unknown exception");
        return false;
    }

    return true;
}
