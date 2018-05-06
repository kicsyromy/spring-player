include (LinuxConfig)
include (macOSConfig)
include (WindowsConfig)

function (libspring_configure_target TARGET_NAME IS_STATIC_LIB PUBLIC_HEADERS PRIVATE_HEADERS SOURCES V_MAJOR V_MINOR V_PATCH)
    if (${IS_STATIC_LIB})
        set (LIBSPRING_USE_IMPORT_SEMANTICS 0)
        add_library (
            ${TARGET_NAME} STATIC
            ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS}
        )
        message (STATUS "Building static library")
    else ()
        set (LIBSPRING_USE_IMPORT_SEMANTICS 1)
        add_library (
            ${TARGET_NAME} SHARED
            ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS}
        )
        message (STATUS "Building shared library")
    endif ()

    configure_file (
        "${PROJECT_SOURCE_DIR}/include/libspring_config.h.in"
        "${PROJECT_BINARY_DIR}/config/libspring_config.h"
        @ONLY
    )

    target_compile_definitions (${TARGET_NAME} PRIVATE "-DSPRING_LIB")

    ## PLATFORM SPECIFIC DEPENDENCIES ##
    libspring_compile_definitions (COMPILE_DEFINITIONS)
    foreach (DEFINITION IN LISTS COMPILE_DEFINITIONS)
        target_compile_definitions (${TARGET_NAME} PRIVATE "${DEFINITION}")
    endforeach ()

    libspring_include_dirs (INCLUDE_DIRS)
    target_include_directories (${TARGET_NAME} PRIVATE "${INCLUDE_DIRS}")

    libspring_libs (LIBS)
    target_link_libraries (${TARGET_NAME} PUBLIC "${LIBS}")

    libspring_compiler_flags (COMPILER_FLAGS)
    set_target_properties (
        ${TARGET_NAME}       PROPERTIES
        COMPILE_FLAGS        "${COMPILER_FLAGS}"
        VERSION              ${V_MAJOR}.${V_MINOR}.${V_PATCH}
        SOVERSION            ${V_MAJOR}.${V_MINOR}
    )
endfunction ()
