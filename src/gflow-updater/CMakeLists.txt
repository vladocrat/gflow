set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER 20)

add_executable(gflow-updater
    main.cpp
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

target_link_libraries(gflow-updater
    PRIVATE
    spdlog::spdlog
    CLI11::CLI11
    utils-cpp
)

gflow_set_warnings(gflow-updater)
gflow_enable_clang_tidy(gflow-updater)
