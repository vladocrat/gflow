// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <sol/sol.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/message_differencer.h>

#include "Bridge.hpp"
#include "ProtoModel.hpp"

namespace {

using google::protobuf::Descriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;

class BridgeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::string error;
        const std::filesystem::path proto = std::filesystem::path(GFLOW_PROTO_DIR) / "types.proto";
        ASSERT_TRUE(m_model.load(proto, {}, &error)) << error;
        m_lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table);
    }

    const Descriptor* descriptor(const std::string& name) const
    {
        for (const auto* d : m_model.messages()) {
            if (d->name() == name) return d;
            for (int i = 0; i < d->nested_type_count(); ++i) {
                if (d->nested_type(i)->name() == name) return d->nested_type(i);
            }
        }
        return nullptr;
    }

    std::unique_ptr<Message> make(const std::string& name) const
    {
        return m_model.newMessage(descriptor(name));
    }

    gflow::ProtoModel m_model;
    sol::state m_lua;
};

TEST_F(BridgeTest, AllScalarTypesRoundTripFromLua)
{
    auto msg = make("Kitchen");
    sol::table t = m_lua.script(R"(
        return {
            i32 = -7,
            i64 = -9223372036854775000,
            u32 = 4000000000,
            u64 = 9223372036854775807,
            s32 = -12345,
            s64 = -123456789012345,
            f32 = 4000000000,
            f64 = 9223372036854775807,
            sf32 = -1,
            sf64 = -1,
            f = 1.5,
            d = 2.5,
            b = true,
            str = "hello \xE2\x9C\x93 unicode",
            raw = "a\0b\xff\xfe",
        }
    )");
    gflow::luaTableToMessage(t, msg.get());

    const Descriptor* d = msg->GetDescriptor();
    const Reflection* r = msg->GetReflection();
    EXPECT_EQ(r->GetInt32(*msg, d->FindFieldByName("i32")), -7);
    EXPECT_EQ(r->GetInt64(*msg, d->FindFieldByName("i64")), -9223372036854775000LL);
    EXPECT_EQ(r->GetUInt32(*msg, d->FindFieldByName("u32")), 4000000000u);
    EXPECT_EQ(r->GetUInt64(*msg, d->FindFieldByName("u64")), 9223372036854775807ULL);
    EXPECT_EQ(r->GetInt32(*msg, d->FindFieldByName("s32")), -12345);
    EXPECT_EQ(r->GetInt64(*msg, d->FindFieldByName("s64")), -123456789012345LL);
    EXPECT_EQ(r->GetUInt64(*msg, d->FindFieldByName("f64")), 9223372036854775807ULL);
    EXPECT_EQ(r->GetFloat(*msg, d->FindFieldByName("f")), 1.5f);
    EXPECT_EQ(r->GetDouble(*msg, d->FindFieldByName("d")), 2.5);
    EXPECT_EQ(r->GetBool(*msg, d->FindFieldByName("b")), true);
    EXPECT_EQ(r->GetString(*msg, d->FindFieldByName("str")), "hello \xE2\x9C\x93 unicode");
    EXPECT_EQ(r->GetString(*msg, d->FindFieldByName("raw")), std::string("a\0b\xff\xfe", 5));
}

TEST_F(BridgeTest, EnumRoundTripsByNameAndRejectsUnknown)
{
    auto msg = make("Kitchen");
    sol::table t = m_lua.script(R"(return { suit = "HEARTS", suits = {"CLUBS", "SPADES"} })");
    gflow::luaTableToMessage(t, msg.get());

    const Descriptor* d = msg->GetDescriptor();
    const Reflection* r = msg->GetReflection();
    EXPECT_EQ(r->GetEnum(*msg, d->FindFieldByName("suit"))->name(), "HEARTS");
    ASSERT_EQ(r->FieldSize(*msg, d->FindFieldByName("suits")), 2);
    EXPECT_EQ(r->GetRepeatedEnum(*msg, d->FindFieldByName("suits"), 0)->name(), "CLUBS");
    EXPECT_EQ(r->GetRepeatedEnum(*msg, d->FindFieldByName("suits"), 1)->name(), "SPADES");

    auto bad = make("Kitchen");
    sol::table badTable = m_lua.script(R"(return { suit = "NOT_A_SUIT" })");
    EXPECT_THROW(gflow::luaTableToMessage(badTable, bad.get()), std::runtime_error);
}

TEST_F(BridgeTest, RecursiveTreeAndRepeatedMessagesRoundTrip)
{
    auto msg = make("Kitchen");
    sol::table t = m_lua.script(R"(
        return {
            tree = { label = "root", children = {
                { label = "a", children = {} },
                { label = "b", children = { { label = "b1", children = {} } } },
            }},
            forest = {
                { label = "f1", children = {} },
                { label = "f2", children = {} },
            },
        }
    )");
    gflow::luaTableToMessage(t, msg.get());

    const Descriptor* d = msg->GetDescriptor();
    const Reflection* r = msg->GetReflection();
    const Message& tree = r->GetMessage(*msg, d->FindFieldByName("tree"));
    const Descriptor* nodeDesc = tree.GetDescriptor();
    const Reflection* nodeR = tree.GetReflection();
    EXPECT_EQ(nodeR->GetString(tree, nodeDesc->FindFieldByName("label")), "root");
    ASSERT_EQ(nodeR->FieldSize(tree, nodeDesc->FindFieldByName("children")), 2);
    const Message& b = nodeR->GetRepeatedMessage(tree, nodeDesc->FindFieldByName("children"), 1);
    EXPECT_EQ(b.GetReflection()->GetString(b, nodeDesc->FindFieldByName("label")), "b");
    ASSERT_EQ(b.GetReflection()->FieldSize(b, nodeDesc->FindFieldByName("children")), 1);

    ASSERT_EQ(r->FieldSize(*msg, d->FindFieldByName("forest")), 2);
}

TEST_F(BridgeTest, MapsWithStringIntNodeAndEnumValues)
{
    auto msg = make("Kitchen");
    sol::table t = m_lua.script(R"(
        return {
            scores = { alice = 10, bob = -5 },
            nodes_by_id = { [1] = { label = "n1", children = {} }, [2] = { label = "n2", children = {} } },
            suit_by_name = { high = "SPADES" },
        }
    )");
    gflow::luaTableToMessage(t, msg.get());

    const Descriptor* d = msg->GetDescriptor();
    const Reflection* r = msg->GetReflection();
    const auto* scoresField = d->FindFieldByName("scores");
    ASSERT_EQ(r->FieldSize(*msg, scoresField), 2);

    const auto* nodesField = d->FindFieldByName("nodes_by_id");
    ASSERT_EQ(r->FieldSize(*msg, nodesField), 2);

    const auto* suitField = d->FindFieldByName("suit_by_name");
    ASSERT_EQ(r->FieldSize(*msg, suitField), 1);
}

TEST_F(BridgeTest, SettingSingleOneofMemberSucceeds)
{
    auto msg = make("Kitchen");
    sol::table t = m_lua.script(R"(return { text = "hello" })");
    EXPECT_NO_THROW(gflow::luaTableToMessage(t, msg.get()));

    const Descriptor* d = msg->GetDescriptor();
    const Reflection* r = msg->GetReflection();
    EXPECT_TRUE(r->HasField(*msg, d->FindFieldByName("text")));
    EXPECT_EQ(r->GetString(*msg, d->FindFieldByName("text")), "hello");
}

TEST_F(BridgeTest, SettingMessageTypedOneofMemberSucceeds)
{
    auto msg = make("Kitchen");
    sol::table t = m_lua.script(R"(return { node_payload = { label = "x", children = {} } })");
    EXPECT_NO_THROW(gflow::luaTableToMessage(t, msg.get()));
}

TEST_F(BridgeTest, TwoIndependentOneofsCanEachBeSetOnce)
{
    auto msg = make("Kitchen");
    sol::table t = m_lua.script(R"(return { text = "hi", flag = true })");
    EXPECT_NO_THROW(gflow::luaTableToMessage(t, msg.get()));
}

TEST_F(BridgeTest, UnsetProto3OptionalScalarIsOmittedFromLuaTable)
{
    auto msg = make("Kitchen"); // maybe_count/maybe_name left unset
    sol::table out = gflow::messageToLuaTable(*msg, m_lua);

    EXPECT_FALSE(out["maybe_count"].valid());
    EXPECT_FALSE(out["maybe_name"].valid());
}

TEST_F(BridgeTest, SetProto3OptionalScalarIsPresentInLuaTable)
{
    auto msg = make("Kitchen");
    const Descriptor* d = msg->GetDescriptor();
    const Reflection* r = msg->GetReflection();
    r->SetInt32(msg.get(), d->FindFieldByName("maybe_count"), 5);

    sol::table out = gflow::messageToLuaTable(*msg, m_lua);
    ASSERT_TRUE(out["maybe_count"].valid());
    EXPECT_EQ(out.get<int>("maybe_count"), 5);
}

// --- full round trip: proto -> lua -> proto -----------------------------
TEST_F(BridgeTest, MessageToLuaToMessageRoundTripsForNonOneofFields)
{
    auto original = make("Kitchen");
    const Descriptor* d = original->GetDescriptor();
    const Reflection* r = original->GetReflection();
    r->SetInt32(original.get(), d->FindFieldByName("i32"), 42);
    r->SetString(original.get(), d->FindFieldByName("str"), "round trip");
    r->SetEnum(original.get(), d->FindFieldByName("suit"), d->FindFieldByName("suit")->enum_type()->FindValueByName("DIAMONDS"));
    Message* tree = r->MutableMessage(original.get(), d->FindFieldByName("tree"));
    tree->GetReflection()->SetString(tree, tree->GetDescriptor()->FindFieldByName("label"), "root");

    sol::table asLua = gflow::messageToLuaTable(*original, m_lua);

    auto rebuilt = make("Kitchen");
    gflow::luaTableToMessage(asLua, rebuilt.get());

    google::protobuf::util::MessageDifferencer differ;
    differ.IgnoreField(d->FindFieldByName("maybe_count"));
    differ.IgnoreField(d->FindFieldByName("maybe_name"));
    std::string diffReport;
    differ.ReportDifferencesToString(&diffReport);
    EXPECT_TRUE(differ.Compare(*original, *rebuilt)) << diffReport;
}

}
