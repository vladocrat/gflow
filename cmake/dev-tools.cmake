
option(GFLOW_ENABLE_CLANG_TIDY "Run clang-tidy on gflow targets during build" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

find_program(GFLOW_CLANG_FORMAT NAMES clang-format HINTS
  "$ENV{ProgramFiles}/LLVM/bin"
  "$ENV{ProgramW6432}/LLVM/bin"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Professional/VC/Tools/Llvm/x64/bin"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Enterprise/VC/Tools/Llvm/x64/bin")
find_program(GFLOW_CLANG_TIDY NAMES clang-tidy HINTS
  "$ENV{ProgramFiles}/LLVM/bin"
  "$ENV{ProgramW6432}/LLVM/bin"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Professional/VC/Tools/Llvm/x64/bin"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Enterprise/VC/Tools/Llvm/x64/bin")

file(GLOB_RECURSE GFLOW_FORMAT_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.h"
  "${CMAKE_SOURCE_DIR}/src/*.hpp"
  "${CMAKE_SOURCE_DIR}/tests/*.cpp"
  "${CMAKE_SOURCE_DIR}/tests/*.h"
  "${CMAKE_SOURCE_DIR}/tests/*.hpp"
)

if(GFLOW_CLANG_FORMAT)
  add_custom_target(format
    COMMAND "${GFLOW_CLANG_FORMAT}" -i --style=file ${GFLOW_FORMAT_SOURCES}
    COMMENT "clang-format: rewriting sources in place"
    VERBATIM)
  add_custom_target(format-check
    COMMAND "${GFLOW_CLANG_FORMAT}" --dry-run -Werror --style=file ${GFLOW_FORMAT_SOURCES}
    COMMENT "clang-format: checking formatting (no changes)"
    VERBATIM)
else()
  message(STATUS "clang-format not found: 'format'/'format-check' targets unavailable")
endif()

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
