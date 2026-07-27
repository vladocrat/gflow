// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include "PathProvider.hpp"

#include <spdlog/spdlog.h>

#ifdef WIN32
#include <shlobj.h>
#include <Windows.h>
#endif

namespace
{

constexpr auto ORG_NAME = "gflow";
constexpr auto PATCH_DIR_NAME = "Update";
constexpr auto ROLLBACK_DIR_NAME = "Rollback";

std::expected<std::filesystem::path, std::error_code> getDir(std::string_view dirName) noexcept
{
    const auto root = paths::getApplicationDataRootDir();

    if (!root) {
        return std::unexpected(root.error());
    }

    auto dir = root.value() / dirName;

    std::error_code errCode;

    if (std::filesystem::exists(dir, errCode)) {
        return dir;
    }

    if (errCode) {
        return std::unexpected(errCode);
    }

    return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));

}

std::expected<std::filesystem::path, std::error_code> createIfNotExists(std::string_view dirName) noexcept
{
    const auto root = paths::getApplicationDataRootDir();

    if (!root) {
        return std::unexpected(root.error());
    }

    auto dir = root.value() / dirName;

    std::error_code errCode;
    std::filesystem::create_directories(dir, errCode);

    if (errCode) {
        return std::unexpected(errCode);
    }

    return dir;
}

}

namespace paths
{

std::expected<std::filesystem::path, std::error_code> getLocalAppData() noexcept
{
#ifdef WIN32
    try {
        PWSTR path {nullptr};

        const auto hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path);

        if (SUCCEEDED(hr)) {
            std::filesystem::path result(path);
            CoTaskMemFree(path);
            return result;
        }

        return std::unexpected(std::error_code{hr, std::system_category()});
    } catch (const std::exception& e) {
        spdlog::error("Failed to get appdata: {}", e.what());
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    } catch (...) {
        spdlog::error("Unkown expection while getting appdata");
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    }

#endif

#ifdef __linux__
    return std::unexpected(std::make_error_code(std::errc::not_supported));
#endif

#ifdef __APPLE__
    return std::unexpected(std::make_error_code(std::errc::not_supported));
#endif
}

std::expected<std::filesystem::path, std::error_code> getApplicationDataRootDir() noexcept
{
    const auto localAppData = getLocalAppData();

    if (!localAppData) {
        return std::unexpected(localAppData.error());
    }

    auto appPath = localAppData.value() / ORG_NAME;

    std::error_code errCode;
    std::filesystem::create_directories(appPath, errCode);

    if (errCode) {
        return std::unexpected(errCode);
    }

    return appPath;
}

std::expected<std::filesystem::path, std::error_code> createTempPatchDir() noexcept
{
    return createIfNotExists(PATCH_DIR_NAME);
}

std::expected<std::filesystem::path, std::error_code> getTempPatchDir() noexcept
{
    return getDir(PATCH_DIR_NAME);
}

std::expected<std::filesystem::path, std::error_code> createTempRollbackDir() noexcept
{
    return createIfNotExists(ROLLBACK_DIR_NAME);
}

std::expected<std::filesystem::path, std::error_code> getTempRollbackDir() noexcept
{
    return getDir(ROLLBACK_DIR_NAME);
}

}
