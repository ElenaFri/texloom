cmake_minimum_required(VERSION 3.25)

project(TexLoom
    VERSION 1.0.0
    DESCRIPTION "A simple Markdown-to-LaTeX converter and text editor"
    LANGUAGES CXX
)

# Enable testing with CTest (must be at project root)
include(CTest)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Code coverage option (enabled for Debug builds)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Code coverage enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Qt specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Widgets
    Concurrent
)

# Find Qt6 Test (optional, for testing)
find_package(Qt6 COMPONENTS Test QUIET)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Source files
set(SOURCES
    src/main.cpp
)

set(HEADERS
)

# UI Component sources
set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/EditorWidget.cpp
    src/ui/PreviewWidget.cpp
    src/ui/ProjectTreeWidget.cpp
)

set(UI_HEADERS
    src/ui/MainWindow.h
    src/ui/EditorWidget.h
    src/ui/PreviewWidget.h
    src/ui/ProjectTreeWidget.h
)

# Core sources
set(CORE_SOURCES
    src/core/ProjectModel.cpp
    src/core/ConversionEngine.cpp
)

set(CORE_HEADERS
    src/core/ProjectModel.h
    src/core/ConversionEngine.h
)

# Service sources (will be added as we create them)
set(SERVICE_SOURCES
    # Future: Additional service classes
)

set(SERVICE_HEADERS
    # Future: Additional service headers
)

# Utility sources (will be added as we create them)
set(UTIL_SOURCES
    # src/utils/FileUtils.cpp
    # src/utils/Settings.cpp
)

set(UTIL_HEADERS
    # src/utils/FileUtils.h
    # src/utils/Settings.h
)

# Resources (Qt resource file)
# set(RESOURCES
#     resources/resources.qrc
# )

# Create core library (for testing)
add_library(texloom_core STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

target_include_directories(texloom_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

target_link_libraries(texloom_core PUBLIC
    Qt6::Core
)

# Create ui library (for testing)
add_library(texloom_ui STATIC
    ${UI_SOURCES}
    ${UI_HEADERS}
)

target_include_directories(texloom_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

target_link_libraries(texloom_ui PUBLIC
    texloom_core
    Qt6::Core
    Qt6::Widgets
)

# Create executable
add_executable(texloom
    ${SOURCES}
    ${HEADERS}
    ${SERVICE_SOURCES}
    ${SERVICE_HEADERS}
    ${UTIL_SOURCES}
    ${UTIL_HEADERS}
    # ${RESOURCES}
)

# Include directories
target_include_directories(texloom PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

# Link libraries
target_link_libraries(texloom PRIVATE
    texloom_core
    texloom_ui
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set_target_properties(texloom PROPERTIES
        WIN32_EXECUTABLE ON
    )
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(texloom PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.texloom.app"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

# Installation rules
install(TARGETS texloom
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Install resources (templates, etc.)
install(DIRECTORY resources/templates
    DESTINATION share/texloom
)

# Testing
if(Qt6Test_FOUND)
    enable_testing()
    add_subdirectory(tests)
endif()

# CPack configuration (for packaging)
set(CPACK_PACKAGE_NAME "TexLoom")
set(CPACK_PACKAGE_VENDOR "TexLoom Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Markdown to LaTeX converter with GUI")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "TexLoom Configuration Summary")
message(STATUS "=============================")
message(STATUS "Version:         ${PROJECT_VERSION}")
message(STATUS "Build type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard:    C++${CMAKE_CXX_STANDARD}")
message(STATUS "Qt version:      ${Qt6_VERSION}")
message(STATUS "Install prefix:  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "System:          ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler:        ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
