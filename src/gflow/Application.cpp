// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include "Application.hpp"

#include <spdlog/spdlog.h>

#include <sol/state.hpp>

#include "gflow-sdk/GRPCClient.hpp"
#include "gflow-sdk/LuaTypeGenerator.hpp"
#include "gflow-sdk/ProtoModel.hpp"

#include "CliParser.hpp"
#include "Registrar.hpp"

namespace gflow
{

struct Application::impl_t
{
    CliParser parser;
    gflow::ProtoModel model;

    int runGenerate(const GenerateOptions& opts)
    {
        std::string error;

        if (!model.load(opts.protoFile, opts.importPaths, &error)) {
            spdlog::error("gflow: {}", error);
            return 1;
        }

        gflow::LuaTypeGenerator::render(model, opts.outputDir);
        return 0;
    }

    int runRun(const RunOptions& opts)
    {
        std::string error;

        if (!model.load(opts.protoFile, opts.importPaths, &error)) {
            spdlog::error("gflow: {}", error);
            return 1;
        }

        gflow::GRPCClient client(opts.address, opts.port);

        sol::state lua;
        lua.open_libraries(
            sol::lib::base,
            sol::lib::string,
            sol::lib::table,
            sol::lib::math,
            sol::lib::os,
            sol::lib::package
        );

        gflow::registerBindings(lua, model, client);

        for (const auto& script : opts.scripts) {
            const auto result = lua.safe_script_file(script.string(), sol::script_pass_on_error);

            if (!result.valid()) {
                const sol::error err = result;
                spdlog::error("{}: {}", script.string(), err.what());
                return 1;
            }
        }

        return 0;
    }
};

Application::Application()
{
    createImpl();
}

Application::~Application() = default;

int Application::run(int argc, char** argv)
{
    const int rc = impl().parser.parse(argc, argv);

    if (rc != 0) {
        spdlog::error("Failed to parse parameters. Parser exited with: {}", rc);
        return rc;
    }

    switch (impl().parser.selected()) {
    case Command::Generate:
        return impl().runGenerate(impl().parser.generateOptions());
    case Command::Run:
        return impl().runRun(impl().parser.runOptions());
    case Command::None:
        spdlog::error("Invalid command");
        return -1;
    }

    return 0;
}

} // namespace gflow
