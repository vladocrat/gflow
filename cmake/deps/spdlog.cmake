include(FetchContent)

set(SPDLOG_INSTALL       OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.15.3
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(spdlog)

if(NOT TARGET spdlog::spdlog)
  message(STATUS "spdlog: NOT configured (no spdlog::spdlog).")
endif()
