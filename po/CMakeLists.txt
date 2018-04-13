project (${APPLICATION_ID}.po)

find_package(Gettext REQUIRED)

set (
    LANGUAGES
    aa ab ae af ak am an ar as ast av ay az ba be bg bh bi bm bn bo br bs ca ce ch ckb co cr cs cu
    cv cy da de dv dz ee el en_AU en_CA en_GB eo es et eu fa ff fi fj fo fr fr_CA fy ga gd gl gn gu
    gv ha he hi ho hr ht hu hy hz ia id ie ig ii ik io is it iu ja jv ka kg ki kj kk kl km kn ko kr
    ks ku kv kw ky la lb lg li ln lo lt lu lv mg mh mi mk ml mn mo mr ms mt my na nb nd ne ng nl nn
    no nr nv ny oc oj om or os pa pi pl ps pt pt_BR qu rm rn ro ru rue rw sa sc sd se sg si sk sl sm
    sma sn so sq sr ss st su sv sw ta te tg th ti tk tl tn to tr ts tt tw ty ug uk ur uz ve vi vo wa
    wo xh yi yo za zh zh_CN zh_HK zh_TW zu
)

# Find sources to use process.
file (
    GLOB GETTEXT_SOURCES
    ${PROJECT_SOURCE_DIR}/../src/*.cpp
    ${PROJECT_SOURCE_DIR}/../src/include/*.tpp
    ${PROJECT_SOURCE_DIR}/../src/ui/*.ui
)

# Create an "update-pot" target.  This will scan the sources and
# update the list of translatable strings.  Note that this will output
# to the source directory, not the build directory.
find_program(XGETTEXT xgettext)
if(NOT "${XGETTEXT}" STREQUAL "XGETTEXT-NOTFOUND")
    add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/${APPLICATION_ID}.pot
        COMMAND ${XGETTEXT}
            -o "${PROJECT_SOURCE_DIR}/${APPLICATION_ID}.pot"
            --add-comments
            --package-name "${APPLICATION_ID}"
            --package-version "0.0.1"
            --msgid-bugs-address "dummy"
            ${GETTEXT_SOURCES}
        DEPENDS ${gettext_source_files})

    add_custom_target(update-pot DEPENDS "${PROJECT_SOURCE_DIR}/${APPLICATION_ID}.pot")
endif()

# Create targets to translate each language's po file into an mo file.
set(mo_files)
foreach(language ${LANGUAGES})
    list(APPEND mo_files "${PROJECT_BINARY_DIR}/${language}.mo")
    add_custom_command(OUTPUT "${language}.mo"
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
            -o "${PROJECT_BINARY_DIR}/${language}.mo"
            "${language}.po"
        DEPENDS "${language}.po"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    install(FILES "${PROJECT_BINARY_DIR}/${language}.mo"
        RENAME "${GETTEXT_PACKAGE}.mo"
        DESTINATION "${CMAKE_INSTALL_LOCALEDIR}/${language}/LC_MESSAGES/")
endforeach(language)

# Create an "update-mo" target which depends on all the mo files.
add_custom_target(update-mo ALL DEPENDS ${mo_files})
