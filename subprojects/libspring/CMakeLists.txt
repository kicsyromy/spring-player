cmake_minimum_required (VERSION 3.0)
project (spring CXX)

list (APPEND CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (ConfigureTarget)
include (Documentation)

set (LIBSPRING_VERSION_MAJOR 0)
set (LIBSPRING_VERSION_MINOR 1)
set (LIBSPRING_VERSION_PATCH 0)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED YES)

option (LIBSPRING_STATIC_LIB "Build static library if set to ON otherwise build shared library." ON)
option (LIBSPRING_DEBUG_LOGS "Build with additional debug logs. CAUTION: Do not use in production." OFF)
option (LIBSPRING_ENABLE_TESTS "Build tests." OFF)
option (LIBSPRING_GENERATE_DOCUMENTATION "Build documentation." OFF)
option (LIBSPRING_BUILD_BINDING_LAYER "Build binding helper library." OFF)

## PRIVATE HEADERS ##
file (GLOB_RECURSE LIBSPRING_PRIVATE_HEADERS "${PROJECT_SOURCE_DIR}/src/include/*.h")
source_group ("Header Files\\Private" FILES ${LIBSPRING_PRIVATE_HEADERS})

## PUBLIC HEADERS ##
file (GLOB_RECURSE LIBSPRING_PUBLIC_HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
source_group ("Header Files\\Public" FILES ${LIBSPRING_PUBLIC_HEADERS})

## SOURCES ##
file (GLOB_RECURSE LIBSPRING_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
source_group ("Source Files" FILES ${LIBSPRING_SOURCES})

## INLINE SOURCES ##
file (GLOB_RECURSE LIBSPRING_INLINE_SOURCES "${PROJECT_SOURCE_DIR}/src/*.tpp")
source_group ("Source Files" FILES ${LIBSPRING_INLINE_SOURCES})

libspring_configure_target (
    ${PROJECT_NAME} ${LIBSPRING_STATIC_LIB}
    "${LIBSPRING_PUBLIC_HEADERS}" "${LIBSPRING_PRIVATE_HEADERS}" "${LIBSPRING_SOURCES}"
    ${LIBSPRING_VERSION_MAJOR} ${LIBSPRING_VERSION_MINOR} ${LIBSPRING_VERSION_PATCH}
)

if (LIBSPRING_DEBUG_LOGS)
    target_compile_definitions (${PROJECT_NAME} PRIVATE "-DLIBSPRING_LOG_DEBUG")
    message (STATUS "Building with debug logging")
endif ()

## PUBLIC HEADERS ##
target_include_directories (${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories (${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}/config")

## PRIVATE HEADERS ##
target_include_directories (${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src/include")

## INLINE SOURCES ##
target_include_directories (${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")

## C++ FORMAT ##
target_compile_definitions (${PROJECT_NAME} PUBLIC "-DFMT_HEADER_ONLY")
target_include_directories (${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/3rdparty/fmt")

## SEQUENTIAL ##
target_include_directories (${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/3rdparty/sequential")
target_include_directories (${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/3rdparty/sequential/formats")

libspring_generate_documentation (${LIBSPRING_GENERATE_DOCUMENTATION})

install (
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)

install (
    FILES "${LIBSPRING_PUBLIC_HEADERS}"
    DESTINATION include/spring
)

install (
    FILES "${PROJECT_BINARY_DIR}/config/libspring_config.h"
    DESTINATION include/spring
)
