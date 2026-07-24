cmake_minimum_required(VERSION 3.14)

project(gflow
  VERSION 0.1.0
  DESCRIPTION "Drive a gRPC server from Lua scripts via runtime proto reflection"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GFLOW ${CMAKE_CURRENT_SOURCE_DIR}/src/gflow)
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/proto)
set(LUA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/lua)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()


option(GFLOW_BUILD_MOCK_SERVER "Build the synthetic EchoService mock server" ON)
option(GFLOW_BUILD_TESTS "Build the gflow test target" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(dev-tools)
include(warnings)

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

target_include_directories(gflow PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(gflow PRIVATE
    GFLOW_PROTO_DIR="${CMAKE_CURRENT_SOURCE_DIR}/proto"
)

gflow_set_warnings(gflow)
gflow_enable_clang_tidy(gflow)

if(GFLOW_BUILD_TESTS)    
  include(deps/gtest)
  add_subdirectory(tests)
endif()
