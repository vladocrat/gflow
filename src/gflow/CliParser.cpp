#include "CliParser.hpp"

#include <limits>

#include <CLI/CLI.hpp>

namespace gflow
{

struct CliParser::impl_t
{
    CLI::App app {"gflow - drive a gRPC server from Lua via runtime proto reflection"};

    GenerateOptions generate;
    RunOptions run;
    Command selected {Command::None};

    void setup()
    {
        app.require_subcommand(1);

        auto* generateCmd = app.add_subcommand("generate", "Generate LuaCATS descriptions");
        generateCmd->add_option("--proto", generate.protoFile, "Path to a .proto file")
            ->required()
            ->check(CLI::ExistingFile);
        generateCmd
            ->add_option("--import-paths", generate.importPaths, "Additional .proto directories for imports lookup")
            ->check(CLI::ExistingDirectory);
        generateCmd
            ->add_option("--output,-o", generate.outputDir, "LuaCATS file destination, defaults to current directory")
            ->check(CLI::ExistingDirectory);
        generateCmd->callback([this]() {
            selected = Command::Generate;
        });

        auto* runCmd = app.add_subcommand("run", "Run scenarious through LUA scripts");
        runCmd->add_option("--address,-a", run.address, "Host address in an IPv4 format")
            ->capture_default_str();
        runCmd->add_option("--port,-p", run.port, "Host address port")
            ->capture_default_str()
            ->check(CLI::Range(std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max()));
        runCmd->add_option("--proto", run.protoFile, "Path to a .proto file")
            ->required()
            ->check(CLI::ExistingFile);
        runCmd->add_option("--import-paths", run.importPaths, "Additional .proto directories for imports lookup")
            ->check(CLI::ExistingDirectory);
        runCmd->add_option("--scripts", run.scripts, "LUA scripts performing scenarious");
        runCmd->callback([this]() {
            selected = Command::Run;
        });
    }
};

CliParser::CliParser()
{
    createImpl();
    impl().setup();
}

CliParser::~CliParser() = default;

int CliParser::parse(int argc, char** argv)
{
    CLI11_PARSE(impl().app, argc, argv);
    return 0;
}

Command CliParser::selected() const
{
    return impl().selected;
}

const GenerateOptions& CliParser::generateOptions() const
{
    return impl().generate;
}

const RunOptions& CliParser::runOptions() const
{
    return impl().run;
}

} // namespace gflow
