cmake_minimum_required(VERSION 3.14)

project(gflow
  VERSION 0.1.0
  DESCRIPTION "Drive a gRPC server from Lua scripts via runtime proto reflection"
  LANGUAGES C CXX)   # C is needed to build the vendored Lua sources

# --- Standard / global settings ----------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# --- Options -----------------------------------------------------------------
# The mock EchoService server used to spike/exercise the dynamic transport
# (DESIGN.md §8.4 / §8.5). Unlike the gflow tool, the mock server is allowed to
# use generated code from proto/echo.proto.
option(GFLOW_BUILD_MOCK_SERVER "Build the synthetic EchoService mock server" ON)

# Unit/integration tests (GoogleTest) live in a separate target, gflow_tests.
option(GFLOW_BUILD_TESTS "Build the gflow test target" ON)

# --- Module path -------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# --- Dev tooling (clang-format / clang-tidy) ---------------------------------
# Independent of the deps below; provides `format`/`format-check` targets and
# the gflow_enable_clang_tidy() helper. Also exports compile_commands.json.
include(dev-tools)

# --- Strict warnings ---------------------------------------------------------
# Provides gflow_set_warnings(<target>): /W4 /WX (MSVC) or -Wall -Wextra -Werror
# (GCC/Clang), applied only to gflow's own targets.
include(warnings)

# --- Dependencies ------------------------------------------------------------
# One module per dependency under cmake/deps/. Each is currently a STUB with a
# contract check; the owner configures the acquisition strategy in each file.
# Include order matters: protobuf -> grpc (needs protobuf) -> lua -> sol2 (needs lua).
# grpc is built from source and ALSO provides protobuf (matching submodule), so
# it must come before protobuf.cmake (which only verifies the protobuf target).
include(deps/grpc)
include(deps/protobuf)
include(deps/lua)
include(deps/sol2)
include(deps/spdlog)   # logging; independent of the core deps above
include(deps/cli11)

# During incremental bring-up the deps above are configured one at a time. A
# target is built only once all the deps it needs exist, so the project keeps
# configuring as each module is filled in.
set(GFLOW_CORE_DEPS_READY TRUE)
foreach(_dep protobuf::libprotobuf gRPC::grpc++ lua::lua sol2::sol2)
  if(NOT TARGET ${_dep})
    set(GFLOW_CORE_DEPS_READY FALSE)
  endif()
endforeach()

# --- gflow executable --------------------------------------------------------
# For now this is a smoke-test entrypoint that only proves the four deps link
# and the Lua VM runs. Real modules (Loader, Bridge, TypedTable, Dispatcher,
# StreamReader, Registrar, TypeEmitter, Runner — DESIGN.md §8) get added here as
# they land, following the milestone order in §11.
if(GFLOW_CORE_DEPS_READY)
  add_executable(gflow
    src/gflow/main.cpp
  )

  target_link_libraries(gflow PRIVATE
    protobuf::libprotobuf
    gRPC::grpc++
    sol2::sol2
    lua::lua
    spdlog::spdlog
    CLI11::CLI11
  )

  target_include_directories(gflow PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

  gflow_set_warnings(gflow)
  gflow_enable_clang_tidy(gflow)
else()
  message(STATUS "gflow executable: SKIPPED (core deps not all configured yet)")
endif()

# --- Mock server (optional) --------------------------------------------------
if(GFLOW_BUILD_MOCK_SERVER)
  # The mock server target + echo.proto codegen will be defined here once the
  # transport spike (milestone 1) starts. Left as a placeholder so configuring
  # with -DGFLOW_BUILD_MOCK_SERVER=ON before that point is a no-op, not an error.
  # add_subdirectory(mock-server)
endif()

# --- Tests (optional) --------------------------------------------------------
# Separate target so the test/GoogleTest dependency never touches the shipping
# gflow binary. As real modules land (Bridge, TypedTable, ...), compile their
# sources into both gflow and gflow_tests (or factor a gflow_core library and
# link it into both) — for now this is a smoke test proving the harness works.
if(GFLOW_BUILD_TESTS)
  include(deps/gtest)   # contract: GTest::gtest, GTest::gtest_main
  if(TARGET GTest::gtest_main)
    enable_testing()

    # Harness-only tests build with just gtest. Tests that exercise other deps
    # are added conditionally below, so the test target builds from the moment
    # gtest is configured — before sol2/lua/etc. land.
    add_executable(gflow_tests
      tests/smoke_test.cpp
    )
    target_link_libraries(gflow_tests PRIVATE GTest::gtest_main)
    target_include_directories(gflow_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    # Lua C-API test: only when lua is configured (verifies lua independently).
    if(TARGET lua::lua)
      target_sources(gflow_tests PRIVATE tests/lua_capi_smoke_test.cpp)
      target_link_libraries(gflow_tests PRIVATE lua::lua)
    endif()

    # Lua/sol2 round-trip test: only when both deps are configured.
    if(TARGET sol2::sol2 AND TARGET lua::lua)
      target_sources(gflow_tests PRIVATE tests/lua_smoke_test.cpp)
      target_link_libraries(gflow_tests PRIVATE sol2::sol2)
    endif()

    gflow_set_warnings(gflow_tests)
    gflow_enable_clang_tidy(gflow_tests)

    # Auto-register each TEST() with CTest. GoogleTest.cmake ships with CMake and
    # works regardless of how gtest was acquired (find_package / FetchContent).
    include(GoogleTest)
    gtest_discover_tests(gflow_tests)
  else()
    message(STATUS "gflow_tests: SKIPPED (gtest not configured)")
  endif()
endif()
