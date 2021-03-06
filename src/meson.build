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

include_dirs = []

headers = files(
    'spring_player.h'
)

sources = files(
    'main.cpp',
    'spring_player.cpp',
)

inline_sources = []

subdir('playback')
subdir('plex')
subdir('ui')
subdir('utility')

project_headers += headers
project_sources += sources
project_sources += inline_sources 

override_options = []

cc_version = meson.get_compiler('cpp').version()
if cc_version.version_compare('>=7.1')
    override_options += 'cpp_std=c++17'
else
    override_options += 'cpp_std=c++14'
endif

libspring = dependency('libspring', fallback : ['libspring', 'libspring_dep'])
executable(
    meson.project_name(),
    sources,
    dependencies: [dependencies, libspring],
    include_directories : [
        include_dirs,
        third_party_include_dirs
    ],
    override_options : override_options,
    install: true
)
