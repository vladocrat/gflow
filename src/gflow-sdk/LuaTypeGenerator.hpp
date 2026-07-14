#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace gflow
{

class ProtoModel;

class LuaTypeGenerator
{
public:
    std::string generate(const ProtoModel& model);
    std::vector<std::filesystem::path> render(const ProtoModel& model, const std::filesystem::path& targetDir);
};

} // namespace gflow
