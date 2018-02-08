find_program (GLIB_COMPILE_RESOURCES_EXECUTABLE NAMES glib-compile-resources)
mark_as_advanced (GLIB_COMPILE_RESOURCES_EXECUTABLE)

function (generate_gresource_file PREFIX FILES OUTPUT_PATH)
    set (OUTPUT_TEXT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<gresources>\n")
    set (OUTPUT_TEXT "${OUTPUT_TEXT}<gresource prefix=\"${PREFIX}\">\n")

    foreach (FILE IN LISTS FILES)
        get_filename_component (FILE_BASENAME ${FILE} NAME)
        if (FILE MATCHES "\.xml$" OR FILE MATCHES "\.ui$")
            set (OUTPUT_TEXT "${OUTPUT_TEXT}<file preprocess=\"xml-stripblanks\" alias=\"${FILE_BASENAME}\">${FILE}</file>\n")
        else ()
            set (OUTPUT_TEXT "${OUTPUT_TEXT}<file alias=\"${FILE_BASENAME}\">${FILE}</file>\n")
        endif ()
    endforeach ()

    set (OUTPUT_TEXT "${OUTPUT_TEXT}</gresource>\n</gresources>\n")

    file (WRITE "${OUTPUT_PATH}" "${OUTPUT_TEXT}")
endfunction ()

function (glib_compile_resources OUTPUT INPUT_FILES)
    set (OUTPUT_DIR ${PROJECT_BINARY_DIR})
    set (OUTPUT_FILES "")

    foreach (INPUT_FILE ${INPUT_FILES})
        get_filename_component (WORKING_DIR ${INPUT_FILE} DIRECTORY)
        get_filename_component (INPUT_FILE ${INPUT_FILE} NAME)
        string (REPLACE ".xml" ".cpp" OUTPUT_FILE ${INPUT_FILE})
        list (APPEND OUTPUT_FILES "${OUTPUT_DIR}/${OUTPUT_FILE}")

        execute_process (
            COMMAND
                ${GLIB_COMPILE_RESOURCES_EXECUTABLE}
                    "--generate-dependencies"
                    ${INPUT_FILE}
            WORKING_DIRECTORY ${WORKING_DIR}
            OUTPUT_VARIABLE INPUT_FILE_DEP
        )

        string (REPLACE ".xml" "" COMMON_NAME ${INPUT_FILE})
        string (REPLACE ".gresource" "" COMMON_NAME ${COMMON_NAME})
        string (REPLACE "." "_" COMMON_NAME ${COMMON_NAME})
        string (REPLACE "-" "_" COMMON_NAME ${COMMON_NAME})

        set (${OUTPUT}_${INPUT_FILE}_COMMON_NAME ${COMMON_NAME} PARENT_SCOPE)

        string (REGEX REPLACE "(\r?\n)" ";" INPUT_FILE_DEP "${INPUT_FILE_DEP}")
        set (INPUT_FILE_DEP_PATH "")
        foreach (DEPENDENCY ${INPUT_FILE_DEP})
            list (APPEND INPUT_FILE_DEP_PATH "${DEPENDENCY}")
        endforeach ()

        add_custom_command (
            OUTPUT "${OUTPUT_DIR}/${OUTPUT_FILE}"
            WORKING_DIRECTORY ${OUTPUT_DIR}
            COMMAND
                ${GLIB_COMPILE_RESOURCES_EXECUTABLE}
            ARGS
                "--generate-source"
                "--c-name=${COMMON_NAME}"
                "--target=${OUTPUT_FILE}"
                "${WORKING_DIR}/${INPUT_FILE}"
            DEPENDS
                ${INPUT_FILE_DEP_PATH}
        )
    endforeach ()
    set (${OUTPUT} ${OUTPUT_FILES} PARENT_SCOPE)
endfunction ()
