#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

#include "gflow-sdk/GRPCClient.hpp"
#include "gflow-sdk/ProtoModel.hpp"
#include "gflow-sdk/LuaTypeGenerator.hpp"

#include "Registrar.hpp"

int main(int argc, char** argv)
{
    CLI::App app{"gflow - drive a gRPC server from Lua via runtime proto reflection"};

    std::string protoFile;
    std::string address;
    std::vector<std::string> importPaths;
    std::vector<std::string> scripts;

    app.add_option("--proto", protoFile, "Path to the .proto file")->required();
    app.add_option("--addr", address, "Server address as host:port")->required();
    app.add_option("--import-path", importPaths, "Additional proto import directories");
    app.add_option("scripts", scripts, "Lua scripts to run, in order")->required();

    CLI11_PARSE(app, argc, argv);

    gflow::ProtoModel model;
    std::string error;
    const std::vector<std::filesystem::path> imports(importPaths.begin(), importPaths.end());

    if (!model.load(protoFile, imports, &error)) {
        spdlog::error("gflow: {}", error);
        return 1;
    }

    gflow::LuaTypeGenerator gen;
    gen.render(model, R"(C:\cpp_projects\gflow\tests\lua\types)");

    const auto colon = address.rfind(':');
    if (colon == std::string::npos) {
        spdlog::error("gflow: --addr must be host:port");
        return 1;
    }
    const std::string host = address.substr(0, colon);
    const auto port = static_cast<uint16_t>(std::stoi(address.substr(colon + 1)));

    gflow::GRPCClient client(host, port);

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::math, sol::lib::os, sol::lib::package);

    gflow::registerBindings(lua, model, client);

    for (const auto& script : scripts) {
        const sol::protected_function_result result = lua.safe_script_file(script, sol::script_pass_on_error);
        if (!result.valid()) {
            const sol::error err = result;
            spdlog::error("gflow: {}: {}", script, err.what());
            return 1;
        }
    }

    return 0;
}
