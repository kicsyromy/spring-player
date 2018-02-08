if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    function (libspring_compile_definitions RESULT)
        set (${RESULT} "-DPLATFORM_LINUX" PARENT_SCOPE)
    endfunction ()

    find_package (PkgConfig QUIET REQUIRED)
    pkg_search_module (CURL REQUIRED libcurl)

    function (libspring_compiler_flags RESULT)
        set (COMPILER_FLAGS "-fPIC")

        list (APPEND COMPILER_FLAGS "${CURL_CFLAGS}")

        string (REPLACE ";" " " COMPILER_FLAGS "${COMPILER_FLAGS}")
        set (${RESULT} ${COMPILER_FLAGS} PARENT_SCOPE)
    endfunction ()

    function (libspring_include_dirs RESULT)
        set (${RESULT} ${CURL_INCLUDE_DIRS} PARENT_SCOPE)
    endfunction ()

    function (libspring_libs RESULT)
        set (${RESULT} ${CURL_LIBRARIES} PARENT_SCOPE)
    endfunction ()
endif ()
