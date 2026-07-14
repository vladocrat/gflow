cmake_minimum_required(VERSION 3.14)

project(gflow
  VERSION 0.1.0
  DESCRIPTION "Drive a gRPC server from Lua scripts via runtime proto reflection"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()


option(GFLOW_BUILD_MOCK_SERVER "Build the synthetic EchoService mock server" ON)
option(GFLOW_BUILD_TESTS "Build the gflow test target" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(dev-tools)
include(warnings)

# --- Dependencies ------------------------------------------------------------
include(deps/grpc)
include(deps/protobuf)
include(deps/lua)
include(deps/sol2)
include(deps/spdlog)
include(deps/cli11)
include(deps/utilscpp)

add_subdirectory(src/gflow-sdk)

add_executable(gflow
  src/gflow/main.cpp
  src/gflow/Application.hpp
  src/gflow/Application.cpp
  src/gflow/CliParser.hpp
  src/gflow/CliParser.cpp
  src/gflow/Registrar.hpp
  src/gflow/Registrar.cpp
)

target_link_libraries(gflow PRIVATE
  protobuf::libprotobuf
  gRPC::grpc++
  sol2::sol2
  lua::lua
  spdlog::spdlog
  CLI11::CLI11
  gflow-sdk
)

target_include_directories(gflow PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_compile_definitions(gflow PRIVATE GFLOW_PROTO_DIR="${CMAKE_CURRENT_SOURCE_DIR}/proto")

gflow_set_warnings(gflow)
gflow_enable_clang_tidy(gflow)


if(GFLOW_BUILD_MOCK_SERVER)
  add_subdirectory(src/echo-server)
endif()


if(GFLOW_BUILD_TESTS)
  include(deps/gtest)
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

    if(TARGET echo-service AND TARGET gflow-sdk)
      target_sources(gflow_tests PRIVATE tests/echo_integration_test.cpp)
      target_link_libraries(gflow_tests PRIVATE echo-service gflow-sdk)
      target_compile_definitions(gflow_tests PRIVATE GFLOW_PROTO_DIR="${CMAKE_CURRENT_SOURCE_DIR}/proto")
    endif()

    if(TARGET gflow-sdk)
      target_sources(gflow_tests PRIVATE tests/lua_type_generator_test.cpp)
      target_link_libraries(gflow_tests PRIVATE gflow-sdk)
      target_compile_definitions(gflow_tests PRIVATE GFLOW_PROTO_DIR="${CMAKE_CURRENT_SOURCE_DIR}/proto")
    endif()

    # CliParser + Application: compile the app sources into the test target so
    # CliParser::parse and Application::run can be exercised directly.
    if(TARGET CLI11::CLI11 AND TARGET sol2::sol2 AND TARGET lua::lua
        AND TARGET spdlog::spdlog AND TARGET echo-service AND TARGET gflow-sdk)
      target_sources(gflow_tests PRIVATE
        tests/cli_parser_test.cpp
        tests/application_run_test.cpp
        src/gflow/CliParser.cpp
        src/gflow/Application.cpp
        src/gflow/Registrar.cpp
      )
      target_link_libraries(gflow_tests PRIVATE
        protobuf::libprotobuf
        gRPC::grpc++
        CLI11::CLI11
        sol2::sol2
        lua::lua
        spdlog::spdlog
        gflow-sdk
        echo-service
      )
      target_compile_definitions(gflow_tests PRIVATE
        GFLOW_PROTO_DIR="${CMAKE_CURRENT_SOURCE_DIR}/proto"
        GFLOW_LUA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/lua"
      )
    endif()

    gflow_set_warnings(gflow_tests)
    gflow_enable_clang_tidy(gflow_tests)

    include(GoogleTest)
    gtest_discover_tests(gflow_tests)
  else()
    message(STATUS "gflow_tests: SKIPPED (gtest not configured)")
  endif()
endif()
