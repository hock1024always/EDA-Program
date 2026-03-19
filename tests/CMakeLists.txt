cmake_minimum_required(VERSION 3.16)

# Find Google Test
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Find Qt6 for GUI tests
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
endif()

# Test executable
add_executable(eda_tests
    test_main.cpp
    test_circuit_model.cpp
    test_bookshelf_parser.cpp
    test_verilog_parser.cpp
    test_placement.cpp
)

target_include_directories(eda_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(eda_tests PRIVATE
    eda_core
    eda_parser
    eda_placer
    GTest::gtest_main
)

# GUI tests (separate executable)
if(BUILD_GUI)
    add_executable(eda_gui_tests
        test_gui_main.cpp
        test_gui.cpp
    )
    
    target_include_directories(eda_gui_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/gui
    )
    
    target_link_libraries(eda_gui_tests PRIVATE
        Qt6::Widgets
        eda_core
        eda_gui  # Link to GUI library
        GTest::gtest_main
    )
    
    # Add GUI tests
    add_test(NAME EDA_GUITests COMMAND eda_gui_tests)
endif()

# Add tests
include(GoogleTest)
gtest_discover_tests(eda_tests)
