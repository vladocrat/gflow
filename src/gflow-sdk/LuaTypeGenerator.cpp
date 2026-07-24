#include "LuaTypeGenerator.hpp"

#include <fstream>

#include <google/protobuf/descriptor.h>
#include <spdlog/spdlog.h>

#include "Bridge.hpp"
#include "ProtoModel.hpp"
#include "Utils.hpp"

namespace gflow
{

namespace
{

// NOLINTBEGIN

struct File
{
    File& meta(const std::string_view name)
    {
        text += std::format("---@meta {}\n", name);
        return *this;
    }

    File& beginClass(const std::string_view name)
    {
        text += std::format("\n---@class (exact) {}", name);
        return *this;
    }

    File& field(const std::string_view name, const std::string_view type, bool optional = true)
    {
        text += std::format("\n---@field {}{} {}", name, optional ? "?" : "", type);
        return *this;
    }

    File& endClass()
    {
        text += "\n";
        return *this;
    }

    File& beginModule()
    {
        text += "\nlocal M = {}\n";
        return *this;
    }

    File& constructor(const std::string_view name)
    {
        text += std::format("\n---@param fields {}\n---@return {}\nfunction M.{}(fields) end\n", name, name, name);
        return *this;
    }

    File& enumType(const std::string_view name, const std::vector<std::string>& values)
    {
        text += std::format("\n---@enum {}\nM.{} = {{\n", name, name);

        for (const auto& value : values) {
            text += std::format("    {} = \"{}\",\n", value, value);
        }

        text += "}\n";
        return *this;
    }

    File& beginService(const std::string_view name)
    {
        currentService = name;
        text += std::format("\nM.{} = {{}}\n", name);
        return *this;
    }

    File& method(const std::string_view name, const std::string_view input, const std::string_view output)
    {
        text += std::format(
            "\n---@param req {}\n---@return {}\nfunction M.{}.{}(req) end\n",
            input,
            output,
            currentService,
            name
        );
        return *this;
    }

    File& moduleReturn()
    {
        text += "\nreturn M\n";
        return *this;
    }

    std::string text;

private:
    std::string_view currentService;
};

// NOLINTEND

} // namespace

std::string LuaTypeGenerator::generate(const ProtoModel& model)
{
    File lua;
    lua.meta(model.name());

    for (const auto* const message : model.messages()) {
        lua.beginClass(message->name());

        for (int i = 0; i < message->field_count(); i++) {
            const auto* const field = message->field(i);
            lua.field(field->name(), luaTypeName(field));
        }

        lua.endClass();
    }

    lua.beginModule();

    for (const auto* const message : model.messages()) {
        lua.constructor(message->name());
    }

    for (const auto* const type : model.enums()) {
        const auto values = std::views::iota(0, type->value_count()) | std::views::transform([type](int i) {
            return std::string(type->value(i)->name());
        }) | to<std::vector<std::string>>();

        lua.enumType(type->name(), values);
    }

    for (const auto* const service : model.services()) {
        lua.beginService(service->name());

        for (int i = 0; i < service->method_count(); i++) {
            const auto* const method = service->method(i);
            lua.method(method->name(), method->input_type()->name(), method->output_type()->name());
        }
    }

    lua.moduleReturn();

    return lua.text;
}

std::vector<std::filesystem::path>
LuaTypeGenerator::render(const ProtoModel& model, const std::filesystem::path& targetDir)
{
    const auto filename = targetDir / (model.name() + ".lua");

    std::ofstream file(filename, std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        spdlog::error("Failed to generate LUA definitions for: {}", filename.string());
        return {};
    }

    file << generate(model);

    return {filename};
}

} // namespace gflow
