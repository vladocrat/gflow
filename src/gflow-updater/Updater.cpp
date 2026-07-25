// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include "Updater.hpp"

#include <spdlog/spdlog.h>

#include "PathProvider.hpp"

bool Updater::execute(const Update& update) noexcept
{
    const auto& to = update.to;
    const auto& from = update.from;

    try {
        const auto rollbackDir = paths::createTempRollbackDir();

        if (!rollbackDir) {
            spdlog::error("Failed to acquire a temp dir, reason: {}", rollbackDir.error().message());
            return false;
        }

        //! Move app files to temp dir
        std::error_code errCode;
        std::filesystem::rename(to, rollbackDir.value(), errCode);

        if (errCode) {
            spdlog::error("Failed to move files to {}, reason: {}", rollbackDir.value(), rollbackDir.error().message());
            return false;
        }

        //! Move patch files to actual dir
        std::filesystem::rename(from, to, errCode);

        if (errCode) {
            spdlog::error("Failed to move files to {}, reason: {}", to, errCode.message());
            return false;
        }

        if (!cleanTemp()) {
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

bool Updater::rollback(const Update& update) noexcept
{
    const auto& to = update.to;

    try {
        const auto rollbackDir = paths::getTempRollbackDir();

        if (!rollbackDir) {
            spdlog::error("Failed to get rollback dir, reason: {}", rollbackDir.error().message());
            return false;
        }

        std::error_code errCode;
        std::filesystem::remove_all(to, errCode);

        if (errCode) {
            spdlog::error("Failed to clean app dir {}, reason: {}", to, errCode.message());
            return false;
        }

        std::filesystem::rename(rollbackDir.value(), to, errCode);

        if (errCode) {
            spdlog::error("Failed to move to app dir {}, reason: {}", to, errCode.message());
        }

        if (!cleanTemp()) {
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

std::error_code Updater::cleanTemp() noexcept
{
    std::error_code errCode;

    const auto patchDir = paths::getTempPatchDir();

    if (!patchDir) {
        return patchDir.error();
    }

    //! Clean up the patch folder
    std::filesystem::remove_all(patchDir.value(), errCode);

    if (errCode) {
        spdlog::warn("Failed to remove patch temp directory {}, reason: {}", patchDir.value(), errCode.message());
    }

    const auto rollbackDir = paths::getTempRollbackDir();

    if (!rollbackDir) {
        return rollbackDir.error();
    }

    //! Clean up the rollback folder
    std::filesystem::remove_all(rollbackDir.value(), errCode);

    if (errCode) {
        spdlog::warn("Failed to remove temp directory {}, reason: {}", rollbackDir.value(), errCode.message());
    }

    return std::error_code{};
}
