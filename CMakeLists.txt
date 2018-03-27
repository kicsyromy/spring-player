project (me.kicsyromy.spring-player)
cmake_minimum_required(VERSION 3.5)

list (APPEND CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (ClangFormat)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.1)
    set (CMAKE_CXX_STANDARD 14)
else ()
    set (CMAKE_CXX_STANDARD 17)
endif ()

set (APPLICATION_ID ${PROJECT_NAME})
set (APPLICATION_PREFIX "/${PROJECT_NAME}")
string (REPLACE "." "/" APPLICATION_PREFIX ${APPLICATION_PREFIX})

set (
    3RDPARTY_INCLUDE_DIRS
    "${PROJECT_SOURCE_DIR}/3rdparty/utfcpp/"
    "${PROJECT_SOURCE_DIR}/3rdparty/concurrentqueue/"
    "${PROJECT_SOURCE_DIR}/3rdparty/sqlite_orm/"
)

add_executable (${APPLICATION_ID} "")

add_subdirectory (src)
target_link_libraries (
    ${APPLICATION_ID}
    ${APPLICATION_ID}.src
)

install (
    TARGETS ${APPLICATION_ID}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

add_subdirectory (resources)
add_subdirectory (schemas)
