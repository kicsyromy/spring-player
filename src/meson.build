dependencies = [
    dependency('glib-2.0', version: '>=2.30.0'),
    dependency('gtk+-3.0', version: '>=3.18.0'),
    dependency('granite'),
    dependency('gstreamer-1.0'),
    dependency('gstreamer-audio-1.0'),
    dependency('gstreamer-app-1.0'),
    dependency('icu-i18n'),
    dependency('sqlite3'),
]

spring_player_sources = [
    'application_settings.cpp',
    'artist_browse_page.cpp',
    'async_queue.cpp',
    'gstreamer_pipeline.cpp',
    'header_bar.cpp',
    'main.cpp',
    'main_window.cpp',
    'page_stack.cpp',
    'page_stack_swicher.cpp',
    'playback_buffer.cpp',
    'playback_list.cpp',
    'playlist_sidebar.cpp',
    'plex_session.cpp',
    'server_setup_dialog.cpp',
    'songs_page.cpp',
    'spring_player.cpp',
    'thumbnail.cpp',
    'track_list_popover.cpp',
    'welcome_page.cpp'
]

include_dirs = include_directories('include')

libspring = dependency('libspring', fallback : ['libspring', 'libspring_dep'])

subdir('ui')

executable(
    meson.project_name(),
    spring_player_sources,
    dependencies: [dependencies, libspring],
    include_directories : [
        include_dirs,
        third_party_include_dirs
    ],
    install: true
)
