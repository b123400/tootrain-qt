cmake_minimum_required(VERSION 3.5)

project(tootrain-qt VERSION 0.8.0 LANGUAGES CXX)

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
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS HttpServer)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mastodon/mastodonoauthwindow.h src/mastodon/mastodonoauthwindow.cpp src/mastodon/mastodonoauthwindow.ui
        src/mastodon/client.h src/mastodon/client.cpp
        src/mastodon/app.h src/mastodon/app.cpp
        src/account.h src/account.cpp
        src/mastodon/account.h src/mastodon/account.cpp
        src/settingwindow.h src/settingwindow.cpp src/settingwindow.ui
        src/settingmanager.h src/settingmanager.cpp
        src/mastodon/status.h src/mastodon/status.cpp
        src/mastodon/streamevent.h src/mastodon/streamevent.cpp
        src/status.h src/status.cpp
        src/dummystatus.h src/dummystatus.cpp
        src/animationstate.h src/animationstate.cpp
        images/icon.rc
        src/mastodon/emoji.h src/mastodon/emoji.cpp
        src/richtextcomponent.h src/richtextcomponent.cpp
        src/statusemoji.h
        src/imagemanager.h src/imagemanager.cpp
        src/statusimageloader.h src/statusimageloader.cpp
        src/mastodon/mastodonsettingwindow.h src/mastodon/mastodonsettingwindow.cpp src/mastodon/mastodonsettingwindow.ui
        src/mastodon/mastodonlist.h src/mastodon/mastodonlist.cpp
        src/urlemoji.h src/urlemoji.cpp
        src/aboutdialog.h src/aboutdialog.cpp src/aboutdialog.ui
        src/misskey/misskeyaccount.h src/misskey/misskeyaccount.cpp
        src/misskey/misskeyauthwindow.h src/misskey/misskeyauthwindow.cpp src/misskey/misskeyauthwindow.ui
        src/misskey/misskeyclient.h src/misskey/misskeyclient.cpp
        src/streamevent.h src/streamevent.cpp
        src/misskey/misskeystreamevent.h src/misskey/misskeystreamevent.cpp
        src/misskey/misskeystatus.h src/misskey/misskeystatus.cpp
        src/misskey/misskeyemoji.h src/misskey/misskeyemoji.cpp
        src/misskey/misskeysettingwindow.h src/misskey/misskeysettingwindow.cpp src/misskey/misskeysettingwindow.ui
        src/misskey/misskeystreamsource.h src/misskey/misskeystreamsource.cpp
)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en ja)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/images/icon.rc")
    set(MACOSX_BUNDLE_ICON_FILE 1024x1024.icns)
    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/images/1024x1024.icns")
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
            images/32x32.png
            images/16x16.png
    )

    set_source_files_properties(${qm_files} PROPERTIES
           MACOSX_PACKAGE_LOCATION "Resources")

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
target_link_libraries(tootrain-qt PRIVATE Qt${QT_VERSION_MAJOR}::HttpServer)

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
