cmake_minimum_required(VERSION 3.25)

# Configure valgrind for memory leak detection
find_program(MEMORYCHECK_COMMAND valgrind)
if(MEMORYCHECK_COMMAND)
    set(MEMORYCHECK_COMMAND_OPTIONS 
        "--leak-check=full"
        "--show-leak-kinds=definite"
        "--error-exitcode=1"
        "--errors-for-leak-kinds=definite"
    )
    message(STATUS "Valgrind found: ${MEMORYCHECK_COMMAND}")
    message(STATUS "  Run memory checks with: ctest -T memcheck")
else()
    message(STATUS "Valgrind not found - memory checks disabled")
endif()

# Test executable macro
macro(add_texloom_test test_name)
    add_executable(${test_name} ${ARGN})
    
    target_link_libraries(${test_name} PRIVATE
        texloom_core
        texloom_ui
        Qt6::Core
        Qt6::Test
        Qt6::Widgets
    )
    
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/services
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/utils
    )
    
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

# Core component tests
add_texloom_test(test_project_model test_project_model.cpp)
add_texloom_test(test_conversion_engine test_conversion_engine.cpp)

# UI component tests
add_texloom_test(test_editor_widget test_editor_widget.cpp)
add_texloom_test(test_preview_widget test_preview_widget.cpp)
add_texloom_test(test_project_tree_widget test_project_tree_widget.cpp)

message(STATUS "Tests configured: test_project_model, test_conversion_engine, test_editor_widget, test_preview_widget, test_project_tree_widget")
