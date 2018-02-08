function (libspring_generate_documentation GENERATE)
    if (${GENERATE})
        find_program (
            SPHINX_EXECUTABLE sphinx-build
            HINTS
            $ENV{SPHINX_DIR}
            PATH_SUFFIXES bin
        )

        find_package (Doxygen QUIET)

        if ((NOT SPHINX_EXECUTABLE MATCHES "SPHINX_EXECUTABLE-NOTFOUND") AND DOXYGEN_FOUND)
            if (NOT DEFINED SPHINX_THEME)
                set (SPHINX_THEME sphinx_rtd_theme)
            endif()

            if (NOT DEFINED SPHINX_THEME_DIR)
                set (SPHINX_THEME_DIR)
            endif()

            set (SPHINX_BINARY_BUILD_DIR "${PROJECT_BINARY_DIR}/_build")
            set (SPHINX_CACHE_DIR        "${PROJECT_BINARY_DIR}/_doctrees")
            set (SPHINX_HTML_DIR         "${PROJECT_BINARY_DIR}/html")

            set (LIBSPRING_PUBLIC_HEADERS_PATH  "${PROJECT_SOURCE_DIR}/include")
            set (LIBSPRING_PRIVATE_HEADERS_PATH "${PROJECT_SOURCE_DIR}/src/include")
            set (LIBSPRING_SOURCES_PATH         "${PROJECT_SOURCE_DIR}/src")
            set (LIBSPRING_DOC_FILES            "${PROJECT_SOURCE_DIR}/docs")

            configure_file (
                "${LIBSPRING_DOC_FILES}/conf.py.in"
                "${SPHINX_BINARY_BUILD_DIR}/conf.py"
                @ONLY
            )

            configure_file (
                "${LIBSPRING_DOC_FILES}/Doxyfile.in"
                "${PROJECT_BINARY_DIR}/Doxyfile"
                @ONLY
            )

            add_custom_target (
                doxygen
                ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
                WORKING_DIRECTORY     ${PROJECT_BINARY_DIR}
                COMMENT "Generating XML API documentation" VERBATIM
            )

            add_custom_target (
                docs ALL
                ${SPHINX_EXECUTABLE}
                    -q -b html
                    -c "${SPHINX_BINARY_BUILD_DIR}"
                    -d "${SPHINX_CACHE_DIR}"
                    "${LIBSPRING_DOC_FILES}"
                    "${SPHINX_HTML_DIR}"
                DEPENDS doxygen
                COMMENT "Building final HTML documentation" VERBATIM
            )
        else ()
            message (
                WARNING
                "Documentation build was requested but Sphinx binary was not found"
            )
        endif ()
    endif ()
endfunction ()
