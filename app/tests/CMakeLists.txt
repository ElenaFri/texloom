cmake_minimum_required(VERSION 3.25)

# Test executable macro
macro(add_texloom_test test_name)
    add_executable(${test_name} ${ARGN})
    
    target_link_libraries(${test_name} PRIVATE
        Qt6::Core
        Qt6::Test
        Qt6::Widgets
    )
    
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/services
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/utils
    )
    
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

# Test files (to be created)
add_texloom_test(test_example test_example.cpp)
# add_texloom_test(test_project test_project.cpp)
# add_texloom_test(test_document test_document.cpp)
# add_texloom_test(test_conversion test_conversion.cpp)

message(STATUS "Tests configured. Run 'ctest' to execute all tests.")
