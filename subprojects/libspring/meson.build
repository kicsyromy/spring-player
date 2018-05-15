project(
    'spring', 'cpp', version: '0.1.0',
    meson_version : '>= 0.46.0',
)

spring_config = configuration_data()
spring_config.set('LIBSPRING_VERSION_MAJOR', 0)
spring_config.set('LIBSPRING_VERSION_MINOR', 1)
spring_config.set('LIBSPRING_VERSION_PATCH', 0)
spring_config.set('LIBSPRING_USE_IMPORT_SEMANTICS', '0')

# Backwards compatibility with CMake
spring_config.set('PROJECT_NAME', meson.project_name())
spring_config.set('PROJECT_BINARY_DIR', meson.current_build_dir())

config_header = configure_file(
    input : 'include/libspring_config.h.in',
    output : '@BASENAME@',
    configuration : spring_config
)

add_project_arguments(
    '-DFMT_HEADER_ONLY',
    language: 'cpp'
)

spring_sources = [
    'src/libspring_error.cpp',
    'src/libspring_http_client.cpp',
    'src/libspring_library_section.cpp',
    'src/libspring_logger.cpp',
    'src/libspring_media_library.cpp',
    'src/libspring_movie_library.cpp',
    'src/libspring_music_album.cpp',
    'src/libspring_music_artist.cpp',
    'src/libspring_music_genre.cpp',
    'src/libspring_music_library.cpp',
    'src/libspring_music_track.cpp',
    'src/libspring_plex_media_server.cpp',
    'src/libspring_tv_show_library.cpp',
    'src/libspring_utilities.cpp',
    'src/libspring_video_library.cpp'
]

spring_public_include_dirs = include_directories('include')
spring_private_include_dirs = include_directories('src/include')
spring_3rdparty_include_dirs = [
    include_directories('3rdparty/fmt'),
    include_directories('3rdparty/sequential'),
    include_directories('3rdparty/sequential/formats')
]

if get_option('generate_documentation')
    subdir('docs')
endif

if get_option('static_lib')
    library_type = 'static_library'
else
    library_type = 'shared_library'
endif

spring_library = build_target(
    meson.project_name(),
    spring_sources,
    include_directories : [
        spring_public_include_dirs,
        spring_private_include_dirs,
        spring_3rdparty_include_dirs
    ],
    target_type : library_type,
    override_options : [ 'cpp_std=c++14' ],
    install : true
)

libspring_dep = declare_dependency(
    link_with : spring_library,
    include_directories : [
        include_directories('.'),
        spring_public_include_dirs,
        spring_3rdparty_include_dirs
    ],
    dependencies : [ dependency('libcurl') ]
)

install_data(
    config_header,
    install_dir : join_paths(get_option('prefix'), 'include')
)

install_subdir(
    'include',
    install_dir : get_option('prefix')
)
