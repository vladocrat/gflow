#include <filesystem>
#include <initializer_list>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "EchoServer.hpp"
#include "gflow/Application.hpp"

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

std::string luaScript(const std::string& name)
{
    return (std::filesystem::path(GFLOW_LUA_DIR) / name).string();
}

TEST(ApplicationRun, NoCommandReturnsError)
{
    Argv args {"gflow"};
    gflow::Application app;

    EXPECT_NE(app.run(args.argc(), args.argv()), 0);
}

TEST(ApplicationRun, UnknownCommandReturnsError)
{
    Argv args {"gflow", "frobnicate"};
    gflow::Application app;

    EXPECT_NE(app.run(args.argc(), args.argv()), 0);
}

TEST(ApplicationRun, GenerateWritesLuaTypeStubs)
{
    const auto outDir = std::filesystem::temp_directory_path() / "gflow_app_generate";
    std::filesystem::create_directories(outDir);

    Argv args {"gflow", "generate", "--proto", protoFile(), "-o", outDir.string()};
    gflow::Application app;

    EXPECT_EQ(app.run(args.argc(), args.argv()), 0);
    EXPECT_TRUE(std::filesystem::exists(outDir / "echo.lua"));

    std::filesystem::remove_all(outDir);
}

TEST(ApplicationRun, GenerateWithMissingProtoReturnsError)
{
    Argv args {"gflow", "generate", "--proto", "does_not_exist.proto"};
    gflow::Application app;

    EXPECT_NE(app.run(args.argc(), args.argv()), 0);
}


TEST(ApplicationRun, RunExecutesUnaryScriptAgainstLiveServer)
{
    gflow::demo::EchoServer server("localhost:0");
    ASSERT_NE(server.port(), 0) << "echo server failed to bind";

    Argv args {"gflow", "run",
               "--proto", protoFile(),
               "-a", "127.0.0.1",
               "-p", std::to_string(server.port()),
               "--scripts", luaScript("echo_hello.lua")};
    gflow::Application app;

    EXPECT_EQ(app.run(args.argc(), args.argv()), 0);
}

} // namespace
