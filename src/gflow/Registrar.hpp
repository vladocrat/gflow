// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <sol/sol.hpp>

namespace gflow
{

class ProtoModel;
class GRPCClient;

void registerBindings(sol::state& lua, ProtoModel& model, GRPCClient& client);

} // namespace gflow
