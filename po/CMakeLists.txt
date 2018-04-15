project (${APPLICATION_ID}.po)

include (Translations)
add_translations_directory (${APPLICATION_ID})
add_translations_catalog (${APPLICATION_ID}
    ../src
    ../src/ui
)

FILE (GLOB PO_FILES ${PROJECT_SOURCE_DIR}/*.po)
add_custom_target (${PROJECT_NAME} SOURCES ${PO_FILES} "${PROJECT_SOURCE_DIR}/${APPLICATION_ID}.pot")
