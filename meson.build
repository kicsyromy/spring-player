project(
    'me.kicsyromy.spring-player', 'c', 'cpp', version: '0.1.0',
    meson_version: '>= 0.46.0',
    default_options : ['cpp_std=c++14']
)

i18n = import('i18n')
gnome = import('gnome')

application_id = meson.project_name()
application_prefix = '/me/kicsyromy/spring-player'

add_project_arguments(
    '-DAPPLICATION_ID="@0@"'.format(application_id),
    '-DAPPLICATION_PREFIX="@0@"'.format(application_prefix),
    language: 'cpp'
)

configuration = configuration_data()
configuration.set('PROJECT_NAME', meson.project_name())
configuration.set('PROJECT_VERSION', meson.project_version())
configuration.set('APPLICATION_ID', application_id)
configuration.set('APPLICATION_PREFIX', application_prefix)
configuration.set('GETTEXT_PACKAGE', meson.project_name())
configuration.set('VERSION', meson.project_version())
configuration.set('PREFIX', get_option('prefix'))

third_party_include_dirs = [
    include_directories('3rdparty/concurrentqueue'),
    include_directories('3rdparty/sqlite_orm'),
    include_directories('3rdparty/utfcpp')
]

subdir('data')
subdir('src')
subdir('po')

meson.add_install_script('meson/postinstall.py')
