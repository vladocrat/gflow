#pragma once

#include <sol/sol.hpp>

namespace gflow
{

class ProtoModel;
class GRPCClient;

void registerBindings(sol::state& lua, ProtoModel& model, GRPCClient& client);

} // namespace gflow
