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

include_dirs = include_directories('include')

headers = files(
    'include/application_settings.h',
    'include/artist_browse_page.h',
    'include/async_queue.h',
    'include/gstreamer_pipeline.h',
    'include/header_bar.h',
    'include/main_window.h',
    'include/page_stack.h',
    'include/page_stack_swicher.h',
    'include/playback_buffer.h',
    'include/playback_list.h',
    'include/playlist_sidebar.h',
    'include/plex_session.h',
    'include/resource_cache.h',
    'include/resource_cache.tpp',
    'include/server_setup_dialog.h',
    'include/songs_page.h',
    'include/spring_player.h',
    'include/thumbnail.h',
    'include/thumbnail_page.h',
    'include/thumbnail_page.tpp',
    'include/thumbnail_widget.h',
    'include/thumbnail_widget.tpp',
    'include/track_list_popover.h',
    'include/welcome_page.h'
)

sources = files(
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
)

inline_sources = files(
    'include/resource_cache.tpp',
    'include/thumbnail_page.tpp',
    'include/thumbnail_widget.tpp'
)

project_headers += headers
project_sources += sources
project_sources += inline_sources 

subdir('ui')

libspring = dependency('libspring', fallback : ['libspring', 'libspring_dep'])
executable(
    meson.project_name(),
    sources,
    dependencies: [dependencies, libspring],
    include_directories : [
        include_dirs,
        third_party_include_dirs
    ],
    install: true
)
