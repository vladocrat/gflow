#pragma once

#include <string>

#include <sol/sol.hpp>

namespace google::protobuf
{
class Message;
class FieldDescriptor;
} // namespace google::protobuf

namespace gflow
{

void luaTableToMessage(const sol::table& table, google::protobuf::Message* message);
sol::table messageToLuaTable(const google::protobuf::Message& message, sol::state_view lua);

std::string luaTypeName(const google::protobuf::FieldDescriptor* field);

} // namespace gflow
