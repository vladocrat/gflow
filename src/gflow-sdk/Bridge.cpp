#include "Bridge.hpp"

#include <cstdint>
#include <format>
#include <set>
#include <stdexcept>
#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

namespace gflow
{

namespace
{

using google::protobuf::FieldDescriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;

std::string toStd(absl::string_view value)
{
    return {value.data(), value.size()};
}

std::string luaScalarType(const FieldDescriptor* field)
{
    switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
    case FieldDescriptor::CPPTYPE_INT64:
    case FieldDescriptor::CPPTYPE_UINT32:
    case FieldDescriptor::CPPTYPE_UINT64:
        return "integer";
    case FieldDescriptor::CPPTYPE_FLOAT:
    case FieldDescriptor::CPPTYPE_DOUBLE:
        return "number";
    case FieldDescriptor::CPPTYPE_BOOL:
        return "boolean";
    case FieldDescriptor::CPPTYPE_STRING:
        return "string";
    case FieldDescriptor::CPPTYPE_ENUM:
        return toStd(field->enum_type()->name());
    case FieldDescriptor::CPPTYPE_MESSAGE:
        return toStd(field->message_type()->name());
    }
    return "any";
}

[[noreturn]] void fail(const std::string& path, const std::string& message)
{
    throw std::runtime_error(path.empty() ? message : path + ": " + message);
}

std::string child(const std::string& path, const std::string& key)
{
    return path.empty() ? key : path + '.' + key;
}

std::string luaValueTypeName(const sol::object& value)
{
    switch (value.get_type()) {
    case sol::type::nil:
        return "nil";
    case sol::type::boolean:
        return "boolean";
    case sol::type::number:
        return "number";
    case sol::type::string:
        return "string";
    case sol::type::table:
        return "table";
    default:
        return "value";
    }
}

int64_t expectInt(const sol::object& value, const std::string& path)
{
    if (value.get_type() != sol::type::number) {
        fail(path, "expected number, got " + luaValueTypeName(value));
    }
    return value.as<int64_t>();
}

double expectNumber(const sol::object& value, const std::string& path)
{
    if (value.get_type() != sol::type::number) {
        fail(path, "expected number, got " + luaValueTypeName(value));
    }
    return value.as<double>();
}

bool expectBool(const sol::object& value, const std::string& path)
{
    if (value.get_type() != sol::type::boolean) {
        fail(path, "expected boolean, got " + luaValueTypeName(value));
    }
    return value.as<bool>();
}

std::string expectString(const sol::object& value, const std::string& path)
{
    if (value.get_type() != sol::type::string) {
        fail(path, "expected string, got " + luaValueTypeName(value));
    }
    return value.as<std::string>();
}

sol::table expectTable(const sol::object& value, const std::string& path, const std::string& forWhat)
{
    if (value.get_type() != sol::type::table) {
        fail(path, "expected table for " + forWhat + ", got " + luaValueTypeName(value));
    }
    return value.as<sol::table>();
}

std::string keyLabel(const sol::object& key)
{
    switch (key.get_type()) {
    case sol::type::string:
        return key.as<std::string>();
    case sol::type::number:
        return std::to_string(key.as<int64_t>());
    default:
        return "<key>";
    }
}

void tableToMessage(const sol::table& table, Message* message, const std::string& path);

void writeScalar(
    Message* message, const FieldDescriptor* field, const sol::object& value, bool repeated, const std::string& path
)
{
    const Reflection* reflection = message->GetReflection();
    switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32: {
        const auto x = static_cast<int32_t>(expectInt(value, path));
        repeated ? reflection->AddInt32(message, field, x) : reflection->SetInt32(message, field, x);
        break;
    }
    case FieldDescriptor::CPPTYPE_INT64: {
        const auto x = expectInt(value, path);
        repeated ? reflection->AddInt64(message, field, x) : reflection->SetInt64(message, field, x);
        break;
    }
    case FieldDescriptor::CPPTYPE_UINT32: {
        const auto x = static_cast<uint32_t>(expectInt(value, path));
        repeated ? reflection->AddUInt32(message, field, x) : reflection->SetUInt32(message, field, x);
        break;
    }
    case FieldDescriptor::CPPTYPE_UINT64: {
        const auto x = static_cast<uint64_t>(expectInt(value, path));
        repeated ? reflection->AddUInt64(message, field, x) : reflection->SetUInt64(message, field, x);
        break;
    }
    case FieldDescriptor::CPPTYPE_FLOAT: {
        const auto x = static_cast<float>(expectNumber(value, path));
        repeated ? reflection->AddFloat(message, field, x) : reflection->SetFloat(message, field, x);
        break;
    }
    case FieldDescriptor::CPPTYPE_DOUBLE: {
        const double x = expectNumber(value, path);
        repeated ? reflection->AddDouble(message, field, x) : reflection->SetDouble(message, field, x);
        break;
    }
    case FieldDescriptor::CPPTYPE_BOOL: {
        const bool x = expectBool(value, path);
        repeated ? reflection->AddBool(message, field, x) : reflection->SetBool(message, field, x);
        break;
    }
    case FieldDescriptor::CPPTYPE_STRING: {
        const std::string x = expectString(value, path);
        repeated ? reflection->AddString(message, field, x) : reflection->SetString(message, field, x);
        break;
    }
    case FieldDescriptor::CPPTYPE_ENUM: {
        const std::string name = expectString(value, path);
        const auto* enumValue  = field->enum_type()->FindValueByName(name);
        if (enumValue == nullptr) {
            fail(path, "unknown enum value '" + name + "' for enum " + toStd(field->enum_type()->name()));
        }
        repeated ? reflection->AddEnum(message, field, enumValue) : reflection->SetEnum(message, field, enumValue);
        break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE:
        fail(path, "internal: message field routed to scalar writer");
    }
}

void writeSingular(Message* message, const FieldDescriptor* field, const sol::object& value, const std::string& path)
{
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
        const sol::table sub = expectTable(value, path, toStd(field->message_type()->name()));
        tableToMessage(sub, message->GetReflection()->MutableMessage(message, field), path);
    } else {
        writeScalar(message, field, value, false, path);
    }
}

void writeMap(Message* message, const FieldDescriptor* field, const sol::object& value, const std::string& path)
{
    const sol::table entries     = expectTable(value, path, "map " + toStd(field->name()));
    const Reflection* reflection = message->GetReflection();
    const auto* keyField         = field->message_type()->map_key();
    const auto* valueField       = field->message_type()->map_value();

    for (const auto& pair : entries) {
        Message* entry = reflection->AddMessage(message, field);
        writeSingular(entry, keyField, pair.first, child(path, "<key>"));
        writeSingular(entry, valueField, pair.second, child(path, keyLabel(pair.first)));
    }
}

void tableToMessage(const sol::table& table, Message* message, const std::string& path)
{
    const auto* descriptor = message->GetDescriptor();
    std::set<const google::protobuf::OneofDescriptor*> oneofsSeen;

    for (const auto& pair : table) {
        if (pair.first.get_type() != sol::type::string) {
            fail(path, "field names must be strings");
        }
        const std::string key = pair.first.as<std::string>();
        const auto* field     = descriptor->FindFieldByName(key);
        if (field == nullptr) {
            fail(path, "unknown field '" + key + "' in " + toStd(descriptor->name()));
        }

        const std::string fieldPath = child(path, key);
        const sol::object& value    = pair.second;

        if (const auto* oneof = field->real_containing_oneof(); oneof != nullptr) {
            if (!oneofsSeen.contains(oneof)) {
                fail(fieldPath, "multiple fields set for oneof '" + toStd(oneof->name()) + "'");
            }
            oneofsSeen.insert(oneof);
        }

        if (field->is_map()) {
            writeMap(message, field, value, fieldPath);
        } else if (field->is_repeated()) {
            const sol::table array = expectTable(value, fieldPath, "repeated " + toStd(field->name()));
            for (std::size_t i = 1; i <= array.size(); ++i) {
                const std::string elementPath = fieldPath + '[' + std::to_string(i) + ']';
                const sol::object element     = array[i];
                if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                    const sol::table sub = expectTable(element, elementPath, toStd(field->message_type()->name()));
                    tableToMessage(sub, message->GetReflection()->AddMessage(message, field), elementPath);
                } else {
                    writeScalar(message, field, element, true, elementPath);
                }
            }
        } else {
            writeSingular(message, field, value, fieldPath);
        }
    }
}

sol::table  messageToTable(const Message& message, sol::state_view& lua);

sol::object singularToLua(const Message& message, const FieldDescriptor* field, sol::state_view& lua)
{
    const Reflection* reflection = message.GetReflection();
    switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
        return sol::make_object(lua, reflection->GetInt32(message, field));
    case FieldDescriptor::CPPTYPE_INT64:
        return sol::make_object(lua, reflection->GetInt64(message, field));
    case FieldDescriptor::CPPTYPE_UINT32:
        return sol::make_object(lua, reflection->GetUInt32(message, field));
    case FieldDescriptor::CPPTYPE_UINT64:
        return sol::make_object(lua, reflection->GetUInt64(message, field));
    case FieldDescriptor::CPPTYPE_FLOAT:
        return sol::make_object(lua, reflection->GetFloat(message, field));
    case FieldDescriptor::CPPTYPE_DOUBLE:
        return sol::make_object(lua, reflection->GetDouble(message, field));
    case FieldDescriptor::CPPTYPE_BOOL:
        return sol::make_object(lua, reflection->GetBool(message, field));
    case FieldDescriptor::CPPTYPE_STRING:
        return sol::make_object(lua, reflection->GetString(message, field));
    case FieldDescriptor::CPPTYPE_ENUM:
        return sol::make_object(lua, toStd(reflection->GetEnum(message, field)->name()));
    case FieldDescriptor::CPPTYPE_MESSAGE:
        return {messageToTable(reflection->GetMessage(message, field), lua)};
    }
    return sol::lua_nil;
}

sol::object repeatedToLua(const Message& message, const FieldDescriptor* field, int index, sol::state_view& lua)
{
    const Reflection* reflection = message.GetReflection();
    switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
        return sol::make_object(lua, reflection->GetRepeatedInt32(message, field, index));
    case FieldDescriptor::CPPTYPE_INT64:
        return sol::make_object(lua, reflection->GetRepeatedInt64(message, field, index));
    case FieldDescriptor::CPPTYPE_UINT32:
        return sol::make_object(lua, reflection->GetRepeatedUInt32(message, field, index));
    case FieldDescriptor::CPPTYPE_UINT64:
        return sol::make_object(lua, reflection->GetRepeatedUInt64(message, field, index));
    case FieldDescriptor::CPPTYPE_FLOAT:
        return sol::make_object(lua, reflection->GetRepeatedFloat(message, field, index));
    case FieldDescriptor::CPPTYPE_DOUBLE:
        return sol::make_object(lua, reflection->GetRepeatedDouble(message, field, index));
    case FieldDescriptor::CPPTYPE_BOOL:
        return sol::make_object(lua, reflection->GetRepeatedBool(message, field, index));
    case FieldDescriptor::CPPTYPE_STRING:
        return sol::make_object(lua, reflection->GetRepeatedString(message, field, index));
    case FieldDescriptor::CPPTYPE_ENUM:
        return sol::make_object(lua, toStd(reflection->GetRepeatedEnum(message, field, index)->name()));
    case FieldDescriptor::CPPTYPE_MESSAGE:
        return {messageToTable(reflection->GetRepeatedMessage(message, field, index), lua)};
    }
    return sol::lua_nil;
}

sol::table messageToTable(const Message& message, sol::state_view& lua)
{
    const auto* descriptor       = message.GetDescriptor();
    const Reflection* reflection = message.GetReflection();
    sol::table out               = lua.create_table();

    for (int i = 0; i < descriptor->field_count(); ++i) {
        const auto* field           = descriptor->field(i);
        const std::string fieldName = toStd(field->name());

        if (field->is_map()) {
            sol::table map         = lua.create_table();
            const auto* keyField   = field->message_type()->map_key();
            const auto* valueField = field->message_type()->map_value();
            const int size         = reflection->FieldSize(message, field);
            for (int j = 0; j < size; ++j) {
                const Message& entry                     = reflection->GetRepeatedMessage(message, field, j);
                map[singularToLua(entry, keyField, lua)] = singularToLua(entry, valueField, lua);
            }
            out[fieldName] = map;
        } else if (field->is_repeated()) {
            sol::table array = lua.create_table();
            const int size   = reflection->FieldSize(message, field);
            for (int j = 0; j < size; ++j) {
                array[j + 1] = repeatedToLua(message, field, j, lua);
            }
            out[fieldName] = array;
        } else {
            const bool optional =
                field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE || field->real_containing_oneof() != nullptr;
            if (optional && !reflection->HasField(message, field)) {
                continue;
            }
            out[fieldName] = singularToLua(message, field, lua);
        }
    }

    return out;
}

} // namespace

void luaTableToMessage(const sol::table& table, google::protobuf::Message* message)
{
    tableToMessage(table, message, toStd(message->GetDescriptor()->name()));
}

sol::table messageToLuaTable(const google::protobuf::Message& message, sol::state_view& lua)
{
    return messageToTable(message, lua);
}

std::string luaTypeName(const google::protobuf::FieldDescriptor* field)
{
    if (field->is_map()) {
        return std::format(
            "table<{}, {}>",
            luaScalarType(field->message_type()->map_key()),
            luaScalarType(field->message_type()->map_value())
        );
    }

    if (field->is_repeated()) {
        return std::format("{}[]", luaScalarType(field));
    }

    return luaScalarType(field);
}

} // namespace gflow
