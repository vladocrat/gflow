#include <cstdio>
#include <iostream>

#include <google/protobuf/stubs/common.h>
#include <grpcpp/grpcpp.h>
#include <sol/sol.hpp>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char** argv) {
  try {
    CLI::App app{"gflow smoke test"};
    bool quiet = false;
    app.add_flag("-q,--quiet", quiet, "suppress the smoke output");
    CLI11_PARSE(app, argc, argv);

    std::cout << "gflow smoke test\n";
    std::cout << "  protobuf : " << (GOOGLE_PROTOBUF_VERSION / 1000000) << "."
              << (GOOGLE_PROTOBUF_VERSION / 1000 % 1000) << "." << (GOOGLE_PROTOBUF_VERSION % 1000)
              << "\n";
    std::cout << "  grpc++   : " << grpc::Version() << "\n";

    sol::state lua;
    lua.open_libraries(sol::lib::base);
    const std::string version = lua.script("return _VERSION").get<std::string>();
    std::cout << "  lua      : " << version << "\n";

    const int answer = lua.script("return 6 * 7").get<int>();
    std::cout << "  sol2     : 6 * 7 = " << answer << "\n";

    spdlog::info("spdlog : logging works (spdlog {}.{}.{})", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR,
                 SPDLOG_VER_PATCH);

    std::cout << "OK - all dependencies linked and the Lua VM ran.\n";
    return 0;
  } catch (const std::exception& e) {
    std::fprintf(stderr, "gflow: fatal: %s\n", e.what());
    return 1;
  } catch (...) {
    std::fputs("gflow: fatal: unknown error\n", stderr);
    return 1;
  }
}
