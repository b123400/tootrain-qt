cmake_minimum_required(VERSION 3.5)

project(tootrain-qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS NetworkAuth)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WebSockets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mastodonoauthwindow.h mastodonoauthwindow.cpp mastodonoauthwindow.ui
        mastodon/client.h mastodon/client.cpp
        mastodon/app.h mastodon/app.cpp
        account.h account.cpp
        mastodon/account.h mastodon/account.cpp
        settingwindow.h settingwindow.cpp settingwindow.ui
        settingmanager.h settingmanager.cpp
        mastodon/status.h mastodon/status.cpp
        mastodon/streamevent.h mastodon/streamevent.cpp
        status.h
        dummystatus.h dummystatus.cpp
        animationstate.h animationstate.cpp
        images/icon.rc
        mastodon/emoji.h mastodon/emoji.cpp
        richtextcomponent.h richtextcomponent.cpp
        statusemoji.h
        imagemanager.h imagemanager.cpp
        statusimageloader.h statusimageloader.cpp
)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en ja)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/images/icon.rc")
    set(MACOSX_BUNDLE_ICON_FILE icon_128.icns)
    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/images/icon_128.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES
           MACOSX_PACKAGE_LOCATION "Resources")

    qt_add_executable(tootrain-qt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${app_icon_resource_windows}
        ${app_icon_macos}
    )
    qt_add_translations(
        tootrain-qt
        QM_FILES_OUTPUT_VARIABLE qm_files
    )
    add_dependencies(tootrain-qt update_translations)

    qt_add_resources(tootrain-qt "app_images"
        FILES
            images/icon_128.png
    )

    set_source_files_properties(${qm_files} PROPERTIES
           MACOSX_PACKAGE_LOCATION "Resources")

    qt_add_resources(tootrain-qt "translations"
        PREFIX "/i18n"
        BASE "${CMAKE_CURRENT_BINARY_DIR}"
        FILES "${qm_files}"
    )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET tootrain-qt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(tootrain-qt SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(tootrain-qt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(tootrain-qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(tootrain-qt PRIVATE Qt${QT_VERSION_MAJOR}::Network)
target_link_libraries(tootrain-qt PRIVATE Qt${QT_VERSION_MAJOR}::NetworkAuth)
target_link_libraries(tootrain-qt PRIVATE Qt${QT_VERSION_MAJOR}::WebSockets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.tootrain-qt)
endif()
set_target_properties(tootrain-qt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS tootrain-qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(tootrain-qt)
endif()
