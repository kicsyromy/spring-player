if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    function (libspring_compile_definitions RESULT)
        set (${RESULT} "-DPLATFORM_MACOS" PARENT_SCOPE)
    endfunction ()

    function (libspring_compiler_flags RESULT)
        set (COMPILER_FLAGS "")
        list (APPEND COMPILER_FLAGS "-x objective-c++")
        list (APPEND COMPILER_FLAGS "-fobjc-arc")

        string (REPLACE ";" " " COMPILER_FLAGS "${COMPILER_FLAGS}")
        set (${RESULT} ${COMPILER_FLAGS} PARENT_SCOPE)
    endfunction ()

    function (libspring_include_dirs RESULT)
        set (${RESULT} "" PARENT_SCOPE)
    endfunction ()

    function (libspring_libs RESULT)
        set (${RESULT} "-framework Cocoa" PARENT_SCOPE)
    endfunction ()
endif ()
