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
set_source_files_properties(${app_Frameworks_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Frameworks")

set(prefix "${PROJECT_NAME}.app/Contents")
set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
set(INSTALL_CMAKE_DIR "${prefix}/Resources")

#get_target_property(_qmake_executable Qt6::/Users/runner/work/CaptureStream/Qt/6.5.3/macos/bin/qmake IMPORTED_LOCATION)
#get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
#find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Widgets Xml)

qt_add_executable(CaptureStream WIN32 MACOSX_BUNDLE ${app_icon_macos} ${app_Frameworks_macos}
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
# based on code from CMake's QtDialog/CMakeLists.txt
macro(install_qt6_plugin _qt_plugin_name _qt_plugins_var _prefix)
    get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
    if(EXISTS "${_qt_plugin_path}")
        get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
        get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
        get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
        set(_qt_plugin_dest "${_prefix}/PlugIns/${_qt_plugin_type}")
        install(FILES "${_qt_plugin_path}"
            DESTINATION "${_qt_plugin_dest}")
        set(${_qt_plugins_var}
            "${${_qt_plugins_var}};\$ENV{DEST_DIR}\${CMAKE_INSTALL_PREFIX}/${_qt_plugin_dest}/${_qt_plugin_file}")
    else()
        message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found")
    endif()
endmacro()

install_qt6_plugin("Qt6::QCocoaIntegrationPlugin" QT_PLUGINS ${prefix})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    "[Paths]\nPlugins = ${_qt_plugin_dir}\n")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    DESTINATION "${INSTALL_CMAKE_DIR}")

# Destination paths below are relative to ${CMAKE_INSTALL_PREFIX}
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR} COMPONENT Runtime
    )

# Note Mac specific extension .app
set(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")

# Directories to look for dependencies
set(DIRS "${CMAKE_BINARY_DIR}")

# Path used for searching by FIND_XXX(), with appropriate suffixes added
if(CMAKE_PREFIX_PATH)
    foreach(dir ${CMAKE_PREFIX_PATH})
        list(APPEND DIRS "${dir}/bin" "${dir}/lib")
    endforeach()
endif()

#add_custom_command(TARGET CaptureStream POST_BUILD
#        COMMAND "${MACDEPLOYQT_EXECUTABLE}"
#        ARGS "CaptureStream.app"
#        COMMENT "Execute macdeployqt to create macOS bundle")
        
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

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
