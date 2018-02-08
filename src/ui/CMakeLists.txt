project (${APPLICATION_ID}.ui)
cmake_minimum_required(VERSION 3.5)

file (GLOB_RECURSE UI "${PROJECT_SOURCE_DIR}/*.ui")
source_group ("UI" FILES ${UI})

file (GLOB_RECURSE CSS "${PROJECT_SOURCE_DIR}/css/*.css")
source_group ("UI" FILES ${CSS})

list (APPEND UI ${CSS})

include (GResource)
generate_gresource_file (
    ${APPLICATION_PREFIX}
    "${UI}"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.gresource.xml"
)
glib_compile_resources (GRESOURCE_UI ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.gresource.xml)

add_library (
    ${PROJECT_NAME} STATIC
    ${UI} ${GRESOURCE_UI}
)

target_include_directories (
    ${PROJECT_NAME} PRIVATE
    ${GTK_INCLUDE_DIRS}
)

target_link_libraries (
    ${PROJECT_NAME} PUBLIC
    ${GTK_LIBRARIES}
)

target_compile_definitions (
    ${PROJECT_NAME} PUBLIC
    "-DGRESOURCE_UI_INIT_RESOURCE=${GRESOURCE_UI_${PROJECT_NAME}.gresource.xml_COMMON_NAME}_get_resource"
    "-DGRESOURCE_UI_DECLARE_RESOURCE=extern GResource *${GRESOURCE_UI_${PROJECT_NAME}.gresource.xml_COMMON_NAME}_get_resource"
)
