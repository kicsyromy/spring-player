sphinx_executable = find_program(
    ['sphinx-build'],
    required : false
)

doxygen_executable = find_program(
    ['doxygen'],
    required : false
)

if (not sphinx_executable.found()) or (not doxygen_executable.found())
    warning('Documentation build was requested but Sphinx or Doxygen binaries are missing')
else
    doc_configuration = configuration_data()
    doc_configuration.merge_from(spring_config)
    doc_configuration.set('SPHINX_THEME',     get_variable('SPHINX_THEME', 'sphinx_rtd_theme'))
    doc_configuration.set('SPHINX_THEME_DIR', get_variable('SPHINX_THEME_DIR', ''))

    doc_configuration.set('DOXYGEN_OUTPUT_PATH',     meson.current_build_dir())

    doc_configuration.set('LIBSPRING_PUBLIC_HEADERS_PATH',  '@0@/../include'.format(meson.current_source_dir()))
    doc_configuration.set('LIBSPRING_PRIVATE_HEADERS_PATH', '@0@/../src/include'.format(meson.current_source_dir()))
    doc_configuration.set('LIBSPRING_SOURCES_PATH',         '@0@/../src'.format(meson.current_source_dir()))

    config_doxyfile = configure_file(
        input : 'Doxyfile.in',
        output : '@BASENAME@',
        configuration : doc_configuration
    )

    config_sphinx_file = configure_file(
        input : 'conf.py.in',
        output : '@BASENAME@',
        configuration : doc_configuration
    )

    target_doxygen = custom_target(
        'doxygen',
        output: 'xml',
        command : [
            doxygen_executable,
            config_doxyfile
        ],
        build_by_default : true
    )

    sphinx_binary_build_dir = '"@0@/_build"'.format(meson.current_build_dir())
    sphinx_cache_dir = '"@0@/_doctrees"'.format(meson.current_build_dir())
    sphinx_html_dir = '"@0@/html"'.format(meson.current_build_dir())
    libspring_doc_files = meson.current_source_dir()

    target_sphinx = custom_target(
        'sphinx',
        output : 'html',
        command : [
            sphinx_executable,
            '-q', '-b', 'html',
            '-c', sphinx_binary_build_dir,
            '-d', sphinx_cache_dir,
            libspring_doc_files,
            sphinx_html_dir
        ],
        depends : target_doxygen,
        build_by_default : true
    )
endif
