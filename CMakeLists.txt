cmake_minimum_required(VERSION 3.16)
project(CaptureStream VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MACOSX_BUNDLE_ICON_FILE icon.icns)
set(app_icon_macos "${CMAKE_SOURCE_DIR}/icon.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Widgets Xml)

qt_add_executable(CaptureStream WIN32 MACOSX_BUNDLE ${app_icon_macos}
    customizedialog.cpp customizedialog.h customizedialog.ui
    downloadmanager.cpp downloadmanager.h
    downloadthread.cpp downloadthread.h
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    messagewindow.cpp messagewindow.h messagewindow.ui
    qt4qt5.h
    scrambledialog.cpp scrambledialog.h scrambledialog.ui
    urldownloader.cpp urldownloader.h
    utility.cpp utility.h
)
target_compile_definitions(CaptureStream PRIVATE
    QT_DISABLE_DEPRECATED_UP_TO=0x050F00
)

target_link_libraries(CaptureStream PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
    Qt::Xml
)

set_target_properties(CaptureStream PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    FRAMEWORK ON
)

# Resources:
set(stylesheet_resource_files
    "stylesheet-mac.qss"
    "stylesheet-ubu.qss"
    "stylesheet-win.qss"
)

qt_add_resources(CaptureStream "stylesheet"
    PREFIX
        "/"
    FILES
        ${stylesheet_resource_files}
)

if(APPLE)
get_target_property(_qmake_executable "Qt6::qmake" IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

add_custom_command(TARGET CaptureStream POST_BUILD
        COMMAND "${MACDEPLOYQT_EXECUTABLE}"
        ARGS "CaptureStream.app"
        COMMENT "Execute macdeployqt to create macOS bundle")
        
install(TARGETS CaptureStream
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET CaptureStream
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
endif()

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
