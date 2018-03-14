project (${APPLICATION_ID}.src.utility CXX)
cmake_minimum_required(VERSION 3.5)

file (GLOB HEADERS "${PROJECT_SOURCE_DIR}/*.h")
source_group ("Headers" FILES ${HEADERS})

file (GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
source_group ("Sources" FILES ${SOURCES})

file (GLOB INLINE_SOURCES "${PROJECT_SOURCE_DIR}/*.inl")
source_group ("Inline Sources" FILES ${SOURCES})

add_library (
    ${PROJECT_NAME} STATIC
    ${HEADERS} ${SOURCES} ${INLINE_SOURCES}
)

target_include_directories (
    ${PROJECT_NAME} PRIVATE
    "${PROJECT_SOURCE_DIR}"
    ${GTK_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_AUDIO_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${LIBICU_INCLUDE_DIRS}
    ${3RDPARTY_INCLUDE_DIRS}
)

target_include_directories (
    ${PROJECT_NAME} PUBLIC
    "${PROJECT_SOURCE_DIR}/../"
)

target_compile_definitions (
    ${PROJECT_NAME} PRIVATE
    "-DAPPLICATION_ID=\"${APPLICATION_ID}\""
    "-DAPPLICATION_PREFIX=\"${APPLICATION_PREFIX}\""
)
