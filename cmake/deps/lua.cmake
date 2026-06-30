set(GFLOW_LUA_VERSION 5.4.7)

add_library(lua::lua STATIC IMPORTED)
set_target_properties(lua::lua PROPERTIES
    IMPORTED_LOCATION "${GFLOW_THIRD_PARTY}/lib/lua_static.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${GFLOW_THIRD_PARTY}/include")
message(STATUS "lua: using prebuilt ${GFLOW_LUA_VERSION} from ${GFLOW_THIRD_PARTY}")
return()

