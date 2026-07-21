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
    std::vector<std::filesystem::path> render(const ProtoModel& model, const std::filesystem::path& targetDir);
};

} // namespace gflow
