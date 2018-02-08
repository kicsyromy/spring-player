project (${APPLICATION_ID}.resources)
cmake_minimum_required(VERSION 3.5)

configure_file (
    "${PROJECT_SOURCE_DIR}/desktop.in"
    "${PROJECT_BINARY_DIR}/${APPLICATION_ID}.desktop"
    @ONLY
)

file (
    COPY        "${PROJECT_SOURCE_DIR}/icons/application.png"
    DESTINATION "${PROJECT_BINARY_DIR}"
)

file (
    RENAME
    "${PROJECT_BINARY_DIR}/application.png"
    "${PROJECT_BINARY_DIR}/${APPLICATION_ID}.png"
)

install (
    FILES "${PROJECT_BINARY_DIR}/${APPLICATION_ID}.desktop"
    DESTINATION share/applications
)

install (
    FILES "${PROJECT_BINARY_DIR}/${APPLICATION_ID}.png"
    DESTINATION share/icons/hicolor/48x48/apps
)

file (GLOB_RECURSE RESOURCES "${PROJECT_SOURCE_DIR}/*.*")
source_group ("Resources" FILES ${RESOURCES})

add_custom_target (${PROJECT_NAME} SOURCES ${RESOURCES})
