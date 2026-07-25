// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>

#include <utils-cpp/pimpl.h>

class UpdateStorageService;

class Updater
{
public:
    Updater(const UpdateStorageService& repository) noexcept;
    virtual ~Updater() noexcept;

    Updater() = delete;
    Updater(const Updater&) = delete;
    Updater(Updater&&) = delete;
    Updater& operator=(const Updater&) = delete;
    Updater& operator=(Updater&&) = delete;

    bool execute(const std::filesystem::path& applicationDir) const noexcept;
    bool rollback(const std::filesystem::path& applicationDir) const noexcept;

private:
    DECLARE_PIMPL
};
