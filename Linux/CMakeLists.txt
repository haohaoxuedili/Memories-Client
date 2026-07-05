cmake_minimum_required(VERSION 3.20)
project(Memories VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---- Options ----
option(BUILD_GUI "Build with GUI support" ON)
option(BUILD_TESTS "Build tests" OFF)
option(USE_SYSTEM_QT "Use system Qt installation" ON)

# ---- Find Qt ----
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS
        Core Gui Widgets Network Quick QuickWidgets
        Concurrent Svg OpenGLWidgets PrintSupport
    )
endif()

# ---- Sources ----
set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/app/Settings.cpp
    src/network/ApiClient.cpp
    src/network/HealthChecker.cpp
    src/network/ImageUploader.cpp
    src/models/UploadQueue.cpp
    src/utils/ImageCache.cpp
    src/utils/Logger.cpp
    src/utils/ThemeManager.cpp
)

set(HEADERS
    src/app/Application.h
    src/app/Settings.h
    src/network/ApiClient.h
    src/network/HealthChecker.h
    src/network/ImageUploader.h
    src/models/ImageModel.h
    src/models/UploadQueue.h
    src/utils/ImageCache.h
    src/utils/Logger.h
)

if(BUILD_GUI)
    list(APPEND SOURCES
        src/ui/MainWindow.cpp
        src/ui/GalleryWidget.cpp
        src/ui/ImageViewer.cpp
        src/ui/UploadDialog.cpp
        src/ui/SettingsDialog.cpp
        src/ui/LoginDialog.cpp
        src/ui/ImageInfoDialog.cpp
        src/ui/FlowLayout.cpp
        src/ui/AppleTitleBar.cpp
        src/ui/MessageBox.cpp
    )
    list(APPEND SOURCES
        resources/resources.qrc
    )
    list(APPEND HEADERS
        src/ui/MainWindow.h
        src/ui/GalleryWidget.h
        src/ui/ImageViewer.h
        src/ui/UploadDialog.h
        src/ui/SettingsDialog.h
        src/ui/LoginDialog.h
        src/ui/ImageInfoDialog.h
        src/ui/MessageBox.h
    )
endif()

# ---- Executable ----
if(BUILD_GUI)
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network
        Qt6::Quick Qt6::QuickWidgets Qt6::Concurrent
        Qt6::Svg Qt6::OpenGLWidgets Qt6::PrintSupport
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # Platform-specific
    if(UNIX AND NOT APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE X11)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            file(GLOB _candidate_libstdcxx_paths "/usr/lib/gcc/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/*/libstdc++.so")
            list(SORT _candidate_libstdcxx_paths COMPARE NATURAL ORDER DESCENDING)
            foreach(_candidate_libstdcxx IN LISTS _candidate_libstdcxx_paths)
                execute_process(
                    COMMAND strings "${_candidate_libstdcxx}"
                    COMMAND grep -q "CXXABI_1.3.15"
                    RESULT_VARIABLE _has_required_cxxabi
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
                if(_has_required_cxxabi EQUAL 0)
                    get_filename_component(_required_libstdcxx_dir "${_candidate_libstdcxx}" DIRECTORY)
                    target_link_directories(${PROJECT_NAME} BEFORE PRIVATE "${_required_libstdcxx_dir}")
                    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-rpath,${_required_libstdcxx_dir}")
                    break()
                endif()
            endforeach()
        endif()
    endif()

    # Install
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    install(FILES resources/memories-client.desktop DESTINATION share/applications)
    install(FILES resources/app-icon.svg DESTINATION share/icons/hicolor/scalable/apps RENAME memories.svg)
else()
    # Headless only
    add_executable(${PROJECT_NAME}-headless ${SOURCES} ${HEADERS})
    target_link_libraries(${PROJECT_NAME}-headless PRIVATE
        Qt6::Core Qt6::Network Qt6::Concurrent
    )
    target_include_directories(${PROJECT_NAME}-headless PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_compile_definitions(${PROJECT_NAME}-headless PRIVATE HEADLESS_ONLY)
endif()

# ---- Packaging ----
set(CPACK_PACKAGE_NAME "memories")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Memories - Image Management Client")
set(CPACK_PACKAGE_CONTACT "idoknow <i@mrcwoods.com>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "idoknow")
set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base-dev (>= 6.2)")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Graphics")
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase >= 6.2")
set(CPACK_GENERATOR "DEB;RPM;TGZ;AppImage")
include(CPack)
