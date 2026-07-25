enable_testing()

add_executable(gflow_tests
    smoke_test.cpp
    lua_capi_smoke_test.cpp
    lua_smoke_test.cpp
    echo_integration_test.cpp
    lua_type_generator_test.cpp
    bridge_test.cpp
    cli_parser_test.cpp
    application_run_test.cpp
    update_storage_service.cpp
)

target_include_directories(gflow_tests
    PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_sources(gflow_tests PRIVATE
    ${GFLOW}/CliParser.cpp
    ${GFLOW}/Application.cpp
    ${GFLOW}/Registrar.cpp
)

target_link_libraries(gflow_tests PRIVATE
    protobuf::libprotobuf
    gRPC::grpc++
    CLI11::CLI11
    sol2::sol2
    lua::lua
    spdlog::spdlog
    GTest::gtest_main
    gflow-sdk
    echo-service
)

target_compile_definitions(gflow_tests PRIVATE
    GFLOW_PROTO_DIR="${PROTO_DIR}"
    GFLOW_LUA_DIR="${LUA_DIR}"
)

if(GFLOW_BUILD_MOCK_SERVER)
  add_subdirectory(echo-server)
endif()

gflow_set_warnings(gflow_tests)
gflow_enable_clang_tidy(gflow_tests)

include(GoogleTest)
gtest_discover_tests(gflow_tests)
