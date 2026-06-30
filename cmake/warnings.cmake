option(GFLOW_WARNINGS_AS_ERRORS "Treat compiler warnings as errors for gflow targets" ON)

function(gflow_set_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE
      /W4
      /permissive-                 # stricter standard conformance
      /external:anglebrackets      # treat <...> includes as external...
      /external:W0                 # ...and silence their warnings
    )
    if(GFLOW_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Wshadow -Wconversion
    )
    if(GFLOW_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()
