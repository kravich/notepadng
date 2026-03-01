if(NOT QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    message(ERROR "Polyfill is for use under Qt5 only")
endif()

# Based on the same name function from FreeCAD (GPLv2)
function(qt_create_resource_file outfile)
    set(oneValueArgs
        OUTPUT_DIRECTORY
        PREFIX)
    set(multiValueArgs
        FILES)
    cmake_parse_arguments(arg "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    string(APPEND QRC "<RCC>\n")
    string(APPEND QRC "  <qresource prefix=\"${arg_PREFIX}\">\n")

    foreach (filepath ${arg_FILES})
        get_filename_component(filename "${filepath}" NAME)
        string(APPEND QRC "    <file alias=\"${filename}\">${filepath}</file>\n")
    endforeach()

    string(APPEND QRC "  </qresource>\n")
    string(APPEND QRC "</RCC>\n")

    file(WRITE ${outfile} ${QRC})
endfunction()

# Partially based on official qt6_add_translations() (BSD-3-Clause)
function(qt_add_translations)
    set(oneValueArgs
        TS_FILE_DIR
        TS_OUTPUT_DIRECTORY
        RESOURCE_PREFIX)
    set(multiValueArgs
        TARGETS
        TS_FILES
        TS_FILE_BASE)
    cmake_parse_arguments(arg "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(targets "${arg_TARGETS}")
    if(NOT "${arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        list(POP_FRONT arg_UNPARSED_ARGUMENTS target)
        list(PREPEND targets "${target}")
        unset(target)
        set(arg_TARGETS ${targets})       # to forward this argument
    endif()
    if(targets STREQUAL "")
        message(FATAL_ERROR "No targets provided.")
    endif()

    if(NOT DEFINED arg_RESOURCE_PREFIX)
        set(arg_RESOURCE_PREFIX "/i18n")
    endif()

    if(NOT DEFINED arg_TS_FILES)
        if(NOT DEFINED arg_TS_FILE_DIR)
            if(DEFINED arg_TS_OUTPUT_DIRECTORY)
                set(arg_TS_FILE_DIR "${arg_TS_OUTPUT_DIRECTORY}")
            else()
                set(arg_TS_FILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
            endif()
        endif()
        if(NOT DEFINED arg_TS_FILE_BASE)
            set(arg_TS_FILE_BASE "${PROJECT_NAME}")
            string(REPLACE " " "-" arg_TS_FILE_BASE "${arg_TS_FILE_BASE}")
        endif()
        set(arg_TS_FILES "")
        foreach(lang IN LISTS QT_I18N_TRANSLATED_LANGUAGES)
            set(ts_file "${arg_TS_FILE_DIR}/${arg_TS_FILE_BASE}_${lang}.ts")
            list(APPEND arg_TS_FILES "${ts_file}")
            set_source_files_properties(${ts_file} ${scope_args} PROPERTIES
                _qt_i18n_translated_language ${lang}
            )
        endforeach()
    endif()

    qt5_add_translation(qmFiles ${arg_TS_FILES})

    add_custom_target(release_translations ALL DEPENDS ${qmFiles})

    foreach(target ${targets})
        add_dependencies(${target} release_translations)

        qt_create_resource_file(${CMAKE_BINARY_DIR}/${target}_translations.qrc PREFIX ${arg_RESOURCE_PREFIX} FILES ${qmFiles})
        qt_add_resources(translation_SRCS ${CMAKE_BINARY_DIR}/${target}_translations.qrc)
        target_sources(${target} PRIVATE ${translation_SRCS})
    endforeach()
endfunction()
