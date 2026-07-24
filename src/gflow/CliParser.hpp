#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <utils-cpp/pimpl.h>

namespace gflow
{

enum class Command : uint8_t
{
    None,
    Generate,
    Run,
};

struct GenerateOptions
{
    std::filesystem::path protoFile;
    std::vector<std::filesystem::path> importPaths;
    std::filesystem::path outputDir {std::filesystem::current_path()};
};

struct RunOptions
{
    std::filesystem::path protoFile;
    std::vector<std::filesystem::path> importPaths;
    std::string address {"127.0.0.1"};
    std::uint16_t port {50051};
    std::vector<std::filesystem::path> scripts;
};

class CliParser final
{
public:
    CliParser();
    ~CliParser();

    CliParser(const CliParser&)            = delete;
    CliParser& operator=(const CliParser&) = delete;
    CliParser(CliParser&&)                 = delete;
    CliParser& operator=(CliParser&&)      = delete;

    int parse(int argc, char** argv);

    Command selected() const;
    const GenerateOptions& generateOptions() const;
    const RunOptions& runOptions() const;

private:
    DECLARE_PIMPL
};

} // namespace gflow
