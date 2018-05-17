# Install application icon
install_data(
    'icons/application.png',
    rename: '@0@.png'.format(application_id),
    install_dir: join_paths(get_option('datadir'), 'icons', 'hicolor', '48x48', 'apps')
)

# Configure and install GSchema file
gschema_file = configure_file (
    input: 'application.gschema.xml.in',
    output: '@0@.gschema.xml'.format(application_id),
    configuration: configuration
)

install_data (
    gschema_file,
    install_dir: join_paths(get_option('prefix'), get_option('datadir'), 'glib-2.0', 'schemas')
)

compile_schemas = find_program('glib-compile-schemas', required: false)
if compile_schemas.found()
    test('Validate schema file', compile_schemas,
        args: ['--strict', '--dry-run', meson.current_source_dir()]
    )
endif

# Install configure, translate and install .desktop file
desktop_in_file = configure_file(
    input: 'application.desktop.in.in',
    output: '@BASENAME@',
    configuration: configuration
)

desktop_file = i18n.merge_file(
    input: desktop_in_file,
    output: '@0@.desktop'.format(application_id),
    po_dir: join_paths(meson.source_root(), 'po', 'extra'),
    type: 'desktop',
    install_dir: join_paths(get_option('datadir'), 'applications'),
    install: true
)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
    test('Validate desktop file', desktop_utils,
        args: [desktop_file]
    )
endif

# Configure, translate, and install AppStream metadata file
appstream_in_file = configure_file(
    input: 'application.appdata.xml.in.in',
    output: '@BASENAME@',
    configuration: configuration
)

appstream_file = i18n.merge_file(
    input: appstream_in_file,
    output: '@0@.appdata.xml'.format(application_id),
    po_dir: join_paths(meson.source_root(), 'po', 'extra'),
    install_dir: join_paths(get_option('datadir'), 'metainfo'),
    install: true
)

appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
    test('Validate appstream file', appstream_util,
        args: ['validate', appstream_file]
    )
endif
