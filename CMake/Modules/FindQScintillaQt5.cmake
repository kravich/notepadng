find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)

find_path(QScintillaQt5_INCLUDE_DIRS
    NAMES Qsci/qsciscintilla.h
    PATH_SUFFIXES qt5
)

find_library(QScintillaQt5_LIBRARIES
    NAMES libqscintilla2_qt5.so.15
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QScintillaQt5
    FOUND_VAR QScintillaQt5_FOUND
    REQUIRED_VARS QScintillaQt5_INCLUDE_DIRS QScintillaQt5_LIBRARIES
)

if(QScintillaQt5_FOUND)
    add_library(QScintillaQt5::QScintillaQt5 SHARED IMPORTED)
    set_target_properties(QScintillaQt5::QScintillaQt5 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${QScintillaQt5_INCLUDE_DIRS}
        INTERFACE_LINK_LIBRARIES ${QScintillaQt5_LIBRARIES}
        IMPORTED_LOCATION ${QScintillaQt5_LIBRARIES}
    )
    target_link_libraries(QScintillaQt5::QScintillaQt5 INTERFACE Qt5::Widgets Qt5::PrintSupport)
endif()
