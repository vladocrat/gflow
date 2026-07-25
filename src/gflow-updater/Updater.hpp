// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>

#include <utils-cpp/pimpl.h>

struct Update
{
    std::filesystem::path from;
    std::filesystem::path to;
};

class StorageRepository;

class Updater
{
public:
    Updater(const StorageRepository& repository) noexcept;
    virtual ~Updater() noexcept;

    Updater() = delete;
    Updater(const Updater&) = delete;
    Updater(Updater&&) = delete;
    Updater& operator=(const Updater&) = delete;
    Updater& operator=(Updater&&) = delete;

    bool execute(const Update&) const noexcept;
    bool rollback(const Update&) const noexcept;

private:
    DECLARE_PIMPL
};
