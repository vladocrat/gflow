include(FetchContent)

set(CLI11_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(CLI11_INSTALL        OFF CACHE BOOL "" FORCE)

FetchContent_Declare(cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG        v2.6.2
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(cli11)

if(NOT TARGET CLI11::CLI11)
  message(STATUS "cli11: NOT configured (no CLI11::CLI11).")
endif()
