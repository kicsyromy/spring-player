function (add_schemas INPUT_FILES)
    find_package (PkgConfig REQUIRED QUIET)

    execute_process (
        COMMAND ${PKG_CONFIG_EXECUTABLE}
            gio-2.0 --variable glib_compile_schemas
        OUTPUT_VARIABLE GLIB_COMPILE_SCHEMAS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    SET (GSETTINGS_DIR "${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/")
    message (STATUS "Schemas will be installed into ${GSETTINGS_DIR}")

    foreach (INPUT_FILE ${INPUT_FILES})
        execute_process (
            COMMAND ${GLIB_COMPILE_SCHEMAS}
                --dry-run --schema-file=${INPUT_FILE}
            ERROR_VARIABLE SCHEMA_INVALID
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if (SCHEMA_INVALID)
            message (FATAL_ERROR "Schema ${INPUT_FILE} validation error: ${SCHEMA_INVALID}")
        else ()
            get_filename_component (WORKING_DIR ${INPUT_FILE} DIRECTORY)
            execute_process (
                COMMAND ${GLIB_COMPILE_SCHEMAS}
                    --schema-file=${INPUT_FILE}
                    --targetdir=${WORKING_DIR}
            )
        endif ()

        install (FILES ${INPUT_FILE} DESTINATION ${GSETTINGS_DIR})
    endforeach ()
endfunction ()
