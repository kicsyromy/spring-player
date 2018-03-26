find_program (
    CLANG_FORMAT clang-format
    NAMES clang-format
          clang-format-3.8
          clang-format-3.9
          clang-format-4.0
          clang-format-5.0
)

if (NOT CLANG_FORMAT MATCHES "CLANG_FORMAT-NOTFOUND")
    add_custom_target (
        clangformat ALL
        COMMAND ${CLANG_FORMAT}
        -style=file
        -i "${PROJECT_SOURCE_DIR}/src/include/*.h"
           "${PROJECT_SOURCE_DIR}/src/include/*.tpp"
           "${PROJECT_SOURCE_DIR}/src/*.cpp"
           "${PROJECT_SOURCE_DIR}/src/utility/*.h"
           "${PROJECT_SOURCE_DIR}/src/utility/*.cpp"
           "${PROJECT_SOURCE_DIR}/src/libspring/include/*.h"
           "${PROJECT_SOURCE_DIR}/src/libspring/src/*.cpp"
           "${PROJECT_SOURCE_DIR}/src/libspring/src/include/*.h"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
endif ()
