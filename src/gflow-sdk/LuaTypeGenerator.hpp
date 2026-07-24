// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace gflow
{

class ProtoModel;

class LuaTypeGenerator
{
public:
    static std::string generate(const ProtoModel& model);
    static std::vector<std::filesystem::path> render(const ProtoModel& model, const std::filesystem::path& targetDir);
};

} // namespace gflow
