find_package(Qt6 COMPONENTS Widgets PrintSupport REQUIRED)

find_path(QScintillaQt6_INCLUDE_DIRS
    NAMES Qsci/qsciscintilla.h
    PATH_SUFFIXES qt6
)

find_library(QScintillaQt6_LIBRARIES
    NAMES libqscintilla2_qt6.so.15
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QScintillaQt6
    FOUND_VAR QScintillaQt6_FOUND
    REQUIRED_VARS QScintillaQt6_INCLUDE_DIRS QScintillaQt6_LIBRARIES
)

if(QScintillaQt6_FOUND)
    add_library(QScintillaQt6::QScintillaQt6 SHARED IMPORTED)
    set_target_properties(QScintillaQt6::QScintillaQt6 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${QScintillaQt6_INCLUDE_DIRS}
        INTERFACE_LINK_LIBRARIES ${QScintillaQt6_LIBRARIES}
        IMPORTED_LOCATION ${QScintillaQt6_LIBRARIES}
    )
    target_link_libraries(QScintillaQt6::QScintillaQt6 INTERFACE Qt6::Widgets Qt6::PrintSupport)
endif()
