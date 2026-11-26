if (NOT DEFINED ROOT)
    get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()
if (NOT DEFINED LIMIT)
    set(LIMIT 500)
endif()

file(GLOB_RECURSE source_files
    RELATIVE "${ROOT}"
    "${ROOT}/src/*.cpp"
    "${ROOT}/src/*.hpp"
    "${ROOT}/src/*.h"
    "${ROOT}/tests/*.cpp"
    "${ROOT}/tests/*.hpp"
    "${ROOT}/tools/*.cpp"
    "${ROOT}/tools/*.hpp"
)
list(SORT source_files)

set(hotspot_count 0)
foreach (relative_path IN LISTS source_files)
    file(READ "${ROOT}/${relative_path}" contents)
    if (contents STREQUAL "")
        set(line_count 0)
    else()
        string(REGEX MATCHALL "\n" newline_matches "${contents}")
        list(LENGTH newline_matches newline_count)
        math(EXPR line_count "${newline_count} + 1")
    endif()

    if (line_count GREATER LIMIT)
        math(EXPR hotspot_count "${hotspot_count} + 1")
        message(STATUS "[hotspot] ${line_count} LOC ${relative_path}")
    endif()
endforeach()

if (hotspot_count EQUAL 0)
    message(STATUS "[hotspot] no source files exceed ${LIMIT} LOC")
else()
    message(STATUS "[hotspot] ${hotspot_count} file(s) exceed ${LIMIT} LOC; this is advisory only")
endif()
