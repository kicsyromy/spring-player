project (${APPLICATION_ID}.src)
cmake_minimum_required(VERSION 3.5)

find_package (PkgConfig REQUIRED QUIET)
pkg_check_modules (GTK REQUIRED gtk+-3.0)
#pkg_check_modules (GRANITE REQUIRED granite)
pkg_check_modules (GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules (GSTREAMER_AUDIO REQUIRED gstreamer-audio-1.0)
pkg_check_modules (GSTREAMER_APP REQUIRED gstreamer-app-1.0)
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
    ${GRANITE_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_AUDIO_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${LIBICU_INCLUDE_DIRS}
    ${3RDPARTY_INCLUDE_DIRS}
)

add_subdirectory (libspring)
add_subdirectory (ui)
add_subdirectory (utility)
target_link_libraries (
    ${PROJECT_NAME} PUBLIC
    ${APPLICATION_ID}.ui
    ${GTK_LIBRARIES}
    ${GRANITE_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_AUDIO_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${LIBICU_LIBRARIES}
    spring
    ${PROJECT_NAME}.utility
    pthread
)

target_compile_definitions (
    ${PROJECT_NAME} PRIVATE
    "-DAPPLICATION_ID=\"${APPLICATION_ID}\""
    "-DAPPLICATION_PREFIX=\"${APPLICATION_PREFIX}\""
)
