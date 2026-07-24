// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include <gtest/gtest.h>

extern "C" {
#include <lauxlib.h>

#include <lua.h>
#include <lualib.h>
}

TEST(LuaCApiSmoke, RunsScript) {
  lua_State* L = luaL_newstate();
  ASSERT_NE(L, nullptr);
  luaL_openlibs(L);

  ASSERT_EQ(luaL_dostring(L, "result = 6 * 7"), LUA_OK);
  lua_getglobal(L, "result");
  const lua_Integer result = lua_tointeger(L, -1);
  EXPECT_EQ(result, 42);

  lua_close(L);
}
