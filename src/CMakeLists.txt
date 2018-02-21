project (${APPLICATION_ID}.src)
cmake_minimum_required(VERSION 3.5)

find_package (PkgConfig REQUIRED QUIET)
pkg_check_modules (GTK REQUIRED gtk+-3.0)
pkg_check_modules (GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules (LIBICU REQUIRED icu-i18n)

file (GLOB HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
source_group ("Headers" FILES ${HEADERS})

file (GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
source_group ("Sources" FILES ${SOURCES})

add_library (
    ${PROJECT_NAME} STATIC
    ${HEADERS} ${SOURCES}
)

target_include_directories (
    ${PROJECT_NAME} PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    ${GTK_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${LIBICU_INCLUDE_DIRS}
    ${3RDPARTY_INCLUDE_DIRS}
)

add_subdirectory (libspring)
add_subdirectory (ui)
target_link_libraries (
    ${PROJECT_NAME} PUBLIC
    ${APPLICATION_ID}.ui
    ${GTK_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${LIBICU_LIBRARIES}
    spring
    pthread
)

target_compile_definitions (
    ${PROJECT_NAME} PRIVATE
    "-DAPPLICATION_ID=\"${APPLICATION_ID}\""
    "-DAPPLICATION_PREFIX=\"${APPLICATION_PREFIX}\""
)
