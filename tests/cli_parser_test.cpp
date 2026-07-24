// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "gflow/CliParser.hpp"

namespace {

class Argv
{
public:
    Argv(std::initializer_list<std::string> args)
    {
        m_storage.reserve(args.size());
        for (const auto& arg : args) {
            m_storage.push_back(arg);
        }
        m_pointers.reserve(m_storage.size() + 1);
        for (auto& arg : m_storage) {
            m_pointers.push_back(arg.data());
        }
        m_pointers.push_back(nullptr);
    }

    int argc() const { return static_cast<int>(m_storage.size()); }
    char** argv() { return m_pointers.data(); }

private:
    std::vector<std::string> m_storage;
    std::vector<char*> m_pointers;
};

std::string protoFile()
{
    return (std::filesystem::path(GFLOW_PROTO_DIR) / "echo.proto").string();
}

std::string protoDir()
{
    return std::filesystem::path(GFLOW_PROTO_DIR).string();
}

TEST(CliParser, NoArgumentsIsError)
{
    Argv args {"gflow"};
    gflow::CliParser parser;

    EXPECT_NE(parser.parse(args.argc(), args.argv()), 0);
    EXPECT_EQ(parser.selected(), gflow::Command::None);
}

TEST(CliParser, UnknownSubcommandIsError)
{
    Argv args {"gflow", "frobnicate"};
    gflow::CliParser parser;

    EXPECT_NE(parser.parse(args.argc(), args.argv()), 0);
    EXPECT_EQ(parser.selected(), gflow::Command::None);
}

// --- generate ---------------------------------------------------------------

TEST(CliParser, ParsesGenerateSubcommand)
{
    Argv args {"gflow", "generate", "--proto", protoFile()};
    gflow::CliParser parser;

    ASSERT_EQ(parser.parse(args.argc(), args.argv()), 0);
    EXPECT_EQ(parser.selected(), gflow::Command::Generate);
    EXPECT_EQ(parser.generateOptions().protoFile, std::filesystem::path(protoFile()));
}

TEST(CliParser, GenerateRequiresProto)
{
    Argv args {"gflow", "generate"};
    gflow::CliParser parser;

    EXPECT_NE(parser.parse(args.argc(), args.argv()), 0);
}

TEST(CliParser, GenerateRejectsMissingProtoFile)
{
    Argv args {"gflow", "generate", "--proto", "does_not_exist.proto"};
    gflow::CliParser parser;

    EXPECT_NE(parser.parse(args.argc(), args.argv()), 0);
}

TEST(CliParser, GenerateCapturesOutputAndImportPaths)
{
    Argv args {"gflow", "generate", "--proto", protoFile(), "-o", protoDir(), "--import-paths", protoDir()};
    gflow::CliParser parser;

    ASSERT_EQ(parser.parse(args.argc(), args.argv()), 0);
    EXPECT_EQ(parser.selected(), gflow::Command::Generate);
    EXPECT_EQ(parser.generateOptions().outputDir, std::filesystem::path(protoDir()));
    ASSERT_EQ(parser.generateOptions().importPaths.size(), 1u);
    EXPECT_EQ(parser.generateOptions().importPaths.front(), std::filesystem::path(protoDir()));
}

TEST(CliParser, GenerateRejectsMissingOutputDirectory)
{
    Argv args {"gflow", "generate", "--proto", protoFile(), "-o", "no_such_dir"};
    gflow::CliParser parser;

    EXPECT_NE(parser.parse(args.argc(), args.argv()), 0);
}

// --- run --------------------------------------------------------------------

TEST(CliParser, ParsesRunSubcommandWithDefaults)
{
    Argv args {"gflow", "run", "--proto", protoFile()};
    gflow::CliParser parser;

    ASSERT_EQ(parser.parse(args.argc(), args.argv()), 0);
    EXPECT_EQ(parser.selected(), gflow::Command::Run);
    EXPECT_EQ(parser.runOptions().address, "127.0.0.1");
    EXPECT_EQ(parser.runOptions().port, 50051);
    EXPECT_TRUE(parser.runOptions().scripts.empty());
}

TEST(CliParser, RunCapturesAddressPortAndScripts)
{
    Argv args {"gflow", "run", "--proto", protoFile(),
               "-a", "0.0.0.0", "-p", "12345",
               "--scripts", "a.lua", "b.lua"};
    gflow::CliParser parser;

    ASSERT_EQ(parser.parse(args.argc(), args.argv()), 0);
    EXPECT_EQ(parser.selected(), gflow::Command::Run);
    EXPECT_EQ(parser.runOptions().address, "0.0.0.0");
    EXPECT_EQ(parser.runOptions().port, 12345);
    ASSERT_EQ(parser.runOptions().scripts.size(), 2u);
    EXPECT_EQ(parser.runOptions().scripts[0], std::filesystem::path("a.lua"));
    EXPECT_EQ(parser.runOptions().scripts[1], std::filesystem::path("b.lua"));
}

TEST(CliParser, RunRequiresProto)
{
    Argv args {"gflow", "run"};
    gflow::CliParser parser;

    EXPECT_NE(parser.parse(args.argc(), args.argv()), 0);
}

TEST(CliParser, RunRejectsPortOutOfRange)
{
    Argv args {"gflow", "run", "--proto", protoFile(), "-p", "70000"};
    gflow::CliParser parser;

    EXPECT_NE(parser.parse(args.argc(), args.argv()), 0);
}

TEST(CliParser, RunCapturesProtoAndImportPaths)
{
    Argv args {"gflow", "run", "--proto", protoFile(), "--import-paths", protoDir()};
    gflow::CliParser parser;

    ASSERT_EQ(parser.parse(args.argc(), args.argv()), 0);
    EXPECT_EQ(parser.runOptions().protoFile, std::filesystem::path(protoFile()));
    ASSERT_EQ(parser.runOptions().importPaths.size(), 1u);
    EXPECT_EQ(parser.runOptions().importPaths.front(), std::filesystem::path(protoDir()));
}

} // namespace
