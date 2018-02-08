project (${APPLICATION_ID}.schemas)
cmake_minimum_required(VERSION 3.5)

file (GLOB_RECURSE SCHEMA_FILES_IN "${PROJECT_SOURCE_DIR}/*.xml.in")
source_group ("Schemas" FILES ${SCHEMA_FILES_IN})

set (SCHEMA_FILES_OUT "")
foreach (SCHEMA_FILE_IN ${SCHEMA_FILES_IN})
    string (REGEX REPLACE "\.in$" "" SCHEMA_FILE_OUT ${SCHEMA_FILE_IN})
    string (REPLACE "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" SCHEMA_FILE_OUT ${SCHEMA_FILE_OUT})

    list (APPEND SCHEMA_FILES_OUT ${SCHEMA_FILE_OUT})

    configure_file (
        "${SCHEMA_FILE_IN}"
        "${SCHEMA_FILE_OUT}"
        @ONLY
    )
endforeach ()

include(GSettings)
add_schemas (${SCHEMA_FILES_OUT})

add_custom_target (${PROJECT_NAME} SOURCES ${SCHEMA_FILES_IN})
