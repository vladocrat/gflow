set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER 20)

add_executable(gflow-updater
    Updater.hpp
    Updater.cpp
    main.cpp
)

target_include_directories(gflow-updater PRIVATE
    "${CMAKE_SOURCE_DIR}/src/gflow-updater-sdk"
)

target_link_libraries(gflow-updater
    PRIVATE
    gflow-updater-sdk
)

gflow_set_warnings(gflow-updater)
gflow_enable_clang_tidy(gflow-updater)
