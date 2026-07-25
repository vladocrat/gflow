add_library(gflow-updater-sdk STATIC
    Updater.hpp
    Updater.cpp
    CliParser.hpp
    CliParser.cpp
    PathProvider.hpp
    PathProvider.cpp
    Storage.hpp
    Storage.cpp
    LocalStorage.hpp
    LocalStorage.cpp
    UpdateStorageService.hpp
    UpdateStorageService.cpp
)

target_link_libraries(gflow-updater-sdk
    PRIVATE
    CLI11::CLI11

    PUBLIC
    spdlog::spdlog
    utils-cpp
)

gflow_set_warnings(gflow-updater-sdk)
gflow_enable_clang_tidy(gflow-updater-sdk)
