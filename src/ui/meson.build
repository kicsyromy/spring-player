spring_player_sources += gnome.compile_resources(
    '@0@.ui.gresource'.format(application_id),
    '@0@.ui.gresource.xml'.format(application_id),
    c_name: '@0@_ui'.format(application_id.underscorify()),
    #target: '@0@.ui.gresource.cpp'.format(application_id)
)

add_project_arguments(
    '-DGRESOURCE_UI_INIT_RESOURCE=@0@_ui_get_resource'.format(application_id.underscorify()),
    '-DGRESOURCE_UI_DECLARE_RESOURCE=extern "C" GResource * @0@_ui_get_resource'.format(application_id.underscorify()),
    language: 'cpp'
)
