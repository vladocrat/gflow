option(GFLOW_ENABLE_CLANG_TIDY "Run clang-tidy on gflow targets during build" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

file(GLOB_RECURSE GFLOW_FORMAT_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/gflow/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/gflow/*.h"
  "${CMAKE_SOURCE_DIR}/src/gflow/*.hpp"
  "${CMAKE_SOURCE_DIR}/src/gflow-sdk/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/gflow-sdk/*.h"
  "${CMAKE_SOURCE_DIR}/src/gflow-sdk/*.hpp"
)

add_custom_target(format
  COMMAND "${GFLOW_CLANG_FORMAT}" -i --style=file ${GFLOW_FORMAT_SOURCES}
  COMMENT "clang-format: rewriting sources in place"
  VERBATIM)
add_custom_target(format-check
  COMMAND "${GFLOW_CLANG_FORMAT}" --dry-run -Werror --style=file ${GFLOW_FORMAT_SOURCES}
  COMMENT "clang-format: checking formatting (no changes)"
  VERBATIM)

function(gflow_enable_clang_tidy target)
  if(NOT GFLOW_ENABLE_CLANG_TIDY)
    return()
  endif()
  if(NOT GFLOW_CLANG_TIDY)
    message(WARNING "GFLOW_ENABLE_CLANG_TIDY=ON but clang-tidy not found; skipping for ${target}")
    return()
  endif()
  set_target_properties(${target} PROPERTIES
    CXX_CLANG_TIDY "${GFLOW_CLANG_TIDY};--quiet")
endfunction()
