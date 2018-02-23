# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

set(cable_buildinfo_template_dir ${CMAKE_CURRENT_LIST_DIR}/buildinfo)

function(cable_add_buildinfo_library NAME)

    set(binary_dir ${CMAKE_CURRENT_BINARY_DIR})

    # Find git here to allow the user to provide hints.
    find_package(Git)

    # Git info target.
    #
    # This target is named <name>-git and is always built.
    # The executed script gitinfo.cmake check git status and updates files
    # containing git information if anything has changed.
    add_custom_target(
        ${NAME}-git
        COMMAND ${CMAKE_COMMAND}
        -DGIT=${GIT_EXECUTABLE}
        -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
        -DBINARY_DIR=${binary_dir}
        -P ${cable_buildinfo_template_dir}/gitinfo.cmake
    )

    add_custom_command(
        COMMENT "Updating ${NAME}:"
        OUTPUT ${binary_dir}/${NAME}.c
        COMMAND ${CMAKE_COMMAND}
        -DNAME=${NAME}
        -DPROJECT_VERSION=${PROJECT_VERSION}
        -DBINARY_DIR=${binary_dir}
        -P ${cable_buildinfo_template_dir}/buildinfo.cmake
        DEPENDS
        ${cable_buildinfo_template_dir}/buildinfo.cmake
        ${cable_buildinfo_template_dir}/buildinfo.c.in
        ${NAME}-git
        ${binary_dir}/git_commit_hash.txt
    )

    string(TIMESTAMP TIMESTAMP)
    configure_file(${cable_buildinfo_template_dir}/buildinfo.h.in ${binary_dir}/${NAME}.h)

    # Add buildinfo library under given name.
    # Make is static and do not build by default until some other target will actually use it.
    add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${binary_dir}/${NAME}.c ${binary_dir}/${NAME}.h)

    target_include_directories(${NAME} PUBLIC ${binary_dir})
endfunction()
