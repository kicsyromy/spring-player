clang_format_executable = find_program(
    [
        'clang-format',
        'clang-format-3.8',
        'clang-format-3.9'
    ],
    required : false
)

if clang_format_executable.found()
    target_clang_format = custom_target(
        'clang-format',
        output : 'src',
        command : [
            clang_format_executable,
            '-style=file',
            '-i',
            project_headers,
            project_sources
        ],
        build_always : false # don't run this just yet
    )
endif