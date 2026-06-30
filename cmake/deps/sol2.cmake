include(FetchContent)

FetchContent_Declare(sol2
  GIT_REPOSITORY https://github.com/ThePhD/sol2.git
  GIT_TAG        v3.5.0
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  no-build
)
FetchContent_MakeAvailable(sol2)

add_library(sol2_headers INTERFACE)

target_include_directories(sol2_headers SYSTEM INTERFACE "${sol2_SOURCE_DIR}/include")
target_link_libraries(sol2_headers INTERFACE lua::lua)

add_library(sol2::sol2 ALIAS sol2_headers)

if(NOT TARGET sol2::sol2)
  message(STATUS "sol2: NOT configured yet (no sol2::sol2).")
endif()
