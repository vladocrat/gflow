find_package(Lua REQUIRED)

if(NOT TARGET lua::lua)
  add_library(lua::lua INTERFACE IMPORTED)
  set_target_properties(lua::lua PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}")
endif()

message(STATUS "lua: using ${LUA_VERSION_STRING} from vcpkg")
