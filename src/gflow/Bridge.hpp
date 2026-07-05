#pragma once

#include <sol/sol.hpp>

namespace google::protobuf
{
class Message;
} // namespace google::protobuf

namespace gflow
{

void luaTableToMessage(const sol::table& table, google::protobuf::Message* message);
sol::table messageToLuaTable(const google::protobuf::Message& message, sol::state_view lua);

} // namespace gflow
