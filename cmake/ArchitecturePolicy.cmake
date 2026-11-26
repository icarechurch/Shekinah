function(shekinah_check_architecture_boundary source_glob forbidden_tokens boundary_name)
    file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${source_glob})
    foreach (source IN LISTS sources)
        file(READ "${source}" contents)
        foreach (forbidden_token IN LISTS forbidden_tokens)
            string(FIND "${contents}" "${forbidden_token}" violation_position)
            if (NOT violation_position EQUAL -1)
                message(FATAL_ERROR
                    "Architecture boundary violation in ${source}: ${boundary_name} cannot include '${forbidden_token}'")
            endif()
        endforeach()
    endforeach()
endfunction()

function(shekinah_enforce_architecture)
    shekinah_check_architecture_boundary(
        "${PROJECT_SOURCE_DIR}/src/core/domain/*.cpp;${PROJECT_SOURCE_DIR}/src/core/domain/*.hpp"
        "shekinah/application/;shekinah/storage/;shekinah/media/;shekinah/ui/;#include <Q"
        "domain")

    shekinah_check_architecture_boundary(
        "${PROJECT_SOURCE_DIR}/src/core/application/*.cpp;${PROJECT_SOURCE_DIR}/src/core/application/*.hpp"
        "shekinah/storage/;shekinah/media/;shekinah/ui/;#include <Q"
        "application")

    shekinah_check_architecture_boundary(
        "${PROJECT_SOURCE_DIR}/src/storage/**/*.cpp;${PROJECT_SOURCE_DIR}/src/storage/**/*.hpp"
        "shekinah/media/;shekinah/ui/;#include <Q"
        "storage")

    shekinah_check_architecture_boundary(
        "${PROJECT_SOURCE_DIR}/src/media/*.cpp;${PROJECT_SOURCE_DIR}/src/media/*.hpp"
        "shekinah/application/;shekinah/storage/;shekinah/ui/"
        "media")
endfunction()
