// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include <gtest/gtest.h>
#include <sol/sol.hpp>

TEST(LuaSmoke, VmRoundTrips) {
  sol::state lua;
  lua.open_libraries(sol::lib::base);
  const int answer = lua.script("return 6 * 7").get<int>();
  EXPECT_EQ(answer, 42);
}
