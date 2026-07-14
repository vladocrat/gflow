#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include "LuaTypeGenerator.hpp"
#include "ProtoModel.hpp"

namespace {

bool contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

std::size_t count(const std::string& haystack, const std::string& needle)
{
    std::size_t total = 0;
    std::size_t pos   = 0;

    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        ++total;
        pos += needle.size();
    }

    return total;
}

class LuaTypeGeneratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::string error;
        const std::filesystem::path proto = std::filesystem::path(GFLOW_PROTO_DIR) / "echo.proto";
        ASSERT_TRUE(m_model.load(proto, {}, &error)) << error;
        m_output = m_gen.generate(m_model);
    }

    gflow::ProtoModel m_model;
    gflow::LuaTypeGenerator m_gen;
    std::string m_output;
};

TEST_F(LuaTypeGeneratorTest, EmitsMetaHeader)
{
    EXPECT_TRUE(contains(m_output, "---@meta echo"));
}

TEST_F(LuaTypeGeneratorTest, MapsScalarTypesToLua)
{
    EXPECT_TRUE(contains(m_output, "count? integer"));
    EXPECT_TRUE(contains(m_output, "ratio? number"));
    EXPECT_TRUE(contains(m_output, "loud? boolean"));
    EXPECT_TRUE(contains(m_output, "message? string"));
    EXPECT_TRUE(contains(m_output, "blob? string"));
    EXPECT_TRUE(contains(m_output, "big? integer"));
}

TEST_F(LuaTypeGeneratorTest, DoesNotLeakProtoTypeNames)
{
    EXPECT_FALSE(contains(m_output, "int32"));
    EXPECT_FALSE(contains(m_output, "int64"));
    EXPECT_FALSE(contains(m_output, "double"));
    EXPECT_FALSE(contains(m_output, "? bytes"));
    EXPECT_FALSE(contains(m_output, "? message"));
    EXPECT_FALSE(contains(m_output, "? enum"));
}

TEST_F(LuaTypeGeneratorTest, UsesMessageTypeNames)
{
    EXPECT_TRUE(contains(m_output, "origin? Point"));
}

TEST_F(LuaTypeGeneratorTest, ModelsRepeatedFields)
{
    EXPECT_TRUE(contains(m_output, "tags? string[]"));
    EXPECT_TRUE(contains(m_output, "path? Point[]"));
}

TEST_F(LuaTypeGeneratorTest, ModelsMapFields)
{
    EXPECT_TRUE(contains(m_output, "counters? table<string, integer>"));
}

TEST_F(LuaTypeGeneratorTest, UsesEnumTypeNameForEnumFields)
{
    EXPECT_TRUE(contains(m_output, "color? Color"));
}

TEST_F(LuaTypeGeneratorTest, EmitsEnumTableWithoutTrailingPipe)
{
    EXPECT_TRUE(contains(m_output, "---@enum Color"));
    EXPECT_TRUE(contains(m_output, "RED = \"RED\""));
    EXPECT_FALSE(contains(m_output, "BLUE|"));
}

TEST_F(LuaTypeGeneratorTest, BindsConstructorToExactClass)
{
    EXPECT_TRUE(contains(m_output, "---@class (exact) Point"));
    EXPECT_TRUE(contains(m_output, "---@param fields Point\n---@return Point\nfunction M.Point(fields) end"));
}

TEST_F(LuaTypeGeneratorTest, BindsServiceMethodAnnotations)
{
    EXPECT_TRUE(contains(m_output, "---@param req EchoRequest\n---@return EchoResponse\nfunction M.EchoService.Echo(req) end"));
}

TEST_F(LuaTypeGeneratorTest, EmitsSingleModuleReturn)
{
    EXPECT_EQ(count(m_output, "\nreturn M"), 1u);
    EXPECT_TRUE(contains(m_output, "M.EchoService = {}"));
    EXPECT_TRUE(contains(m_output, "M.Color = {"));
}

TEST_F(LuaTypeGeneratorTest, RenderWritesFileToTargetDir)
{
    const auto dir = std::filesystem::temp_directory_path() / "gflow_luagen_render";
    std::filesystem::create_directories(dir);

    const auto paths = m_gen.render(m_model, dir);
    ASSERT_EQ(paths.size(), 1u);
    EXPECT_EQ(paths[0], dir / "echo.lua");
    ASSERT_TRUE(std::filesystem::exists(dir / "echo.lua"));

    std::string written;
    {
        std::ifstream in(dir / "echo.lua", std::ios::binary);
        written.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }
    EXPECT_EQ(written, m_output);

    std::filesystem::remove_all(dir);
}

void writeProto(const std::filesystem::path& dir, const std::string& name, const std::string& body)
{
    std::filesystem::create_directories(dir);
    std::ofstream out(dir / name, std::ios::binary);
    out << body;
}

TEST(LuaTypeGeneratorEdge, EmptyModelRendersMetaOnly)
{
    const auto dir = std::filesystem::temp_directory_path() / "gflow_luagen_empty";
    writeProto(dir, "empty.proto", "syntax = \"proto3\";\n");

    gflow::ProtoModel model;
    std::string error;
    ASSERT_TRUE(model.load(dir / "empty.proto", {}, &error)) << error;

    gflow::LuaTypeGenerator gen;
    const std::string out = gen.generate(model);

    EXPECT_TRUE(contains(out, "---@meta"));
    EXPECT_FALSE(contains(out, "---@class"));

    std::filesystem::remove_all(dir);
}

TEST(LuaTypeGeneratorEdge, MultipleServicesProduceSingleReturn)
{
    const auto dir = std::filesystem::temp_directory_path() / "gflow_luagen_multi";
    const std::string body =
        "syntax = \"proto3\";\n"
        "message Req { int32 x = 1; }\n"
        "message Res { int32 y = 1; }\n"
        "service Alpha { rpc Do(Req) returns (Res); }\n"
        "service Beta { rpc Go(Req) returns (Res); }\n";
    writeProto(dir, "multi.proto", body);

    gflow::ProtoModel model;
    std::string error;
    ASSERT_TRUE(model.load(dir / "multi.proto", {}, &error)) << error;

    gflow::LuaTypeGenerator gen;
    const std::string out = gen.generate(model);

    EXPECT_EQ(count(out, "\nreturn M"), 1u);
    EXPECT_TRUE(contains(out, "M.Alpha = {}"));
    EXPECT_TRUE(contains(out, "M.Beta = {}"));

    std::filesystem::remove_all(dir);
}

}
