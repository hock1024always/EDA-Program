cmake_minimum_required(VERSION 3.16)
project(EDA_Integrated VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_GUI "Build GUI application" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Find packages
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
    qt_standard_project_setup()
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library (header-only for data structures)
add_library(eda_core INTERFACE)
target_include_directories(eda_core INTERFACE
    ${CMAKE_SOURCE_DIR}/include/core
)
target_link_libraries(eda_core INTERFACE Eigen3::Eigen)

# Parser library
add_library(eda_parser STATIC
    src/parser/bookshelf_parser.cpp
    src/parser/verilog_parser.cpp
    src/parser/parser_utils.cpp
)
target_include_directories(eda_parser PUBLIC
    ${CMAKE_SOURCE_DIR}/include/parser
    ${CMAKE_SOURCE_DIR}/include/core
)
target_link_libraries(eda_parser PUBLIC eda_core)

# Placer library
add_library(eda_placer STATIC
    src/placer/initial_placer.cpp
    src/placer/global_placer.cpp
    src/placer/placer_base.cpp
)
target_include_directories(eda_placer PUBLIC
    ${CMAKE_SOURCE_DIR}/include/placer
    ${CMAKE_SOURCE_DIR}/include/core
)
target_link_libraries(eda_placer PUBLIC eda_core Eigen3::Eigen)

# Visualizer library
add_library(eda_visualizer STATIC
    src/visualizer/placement_visualizer.cpp
    src/visualizer/circuit_renderer.cpp
)
target_include_directories(eda_visualizer PUBLIC
    ${CMAKE_SOURCE_DIR}/include/visualizer
    ${CMAKE_SOURCE_DIR}/include/core
)
target_link_libraries(eda_visualizer PUBLIC eda_core)

# Utils library
add_library(eda_utils STATIC
    src/utils/math_utils.cpp
    src/utils/file_utils.cpp
    src/utils/logger.cpp
)
target_include_directories(eda_utils PUBLIC
    ${CMAKE_SOURCE_DIR}/include/utils
)

# GUI application
if(BUILD_GUI)
    qt_add_executable(eda_gui
        src/gui/main.cpp
        src/gui/main_window.cpp
        src/gui/circuit_canvas.cpp
        src/gui/placement_view.cpp
        src/gui/algorithm_panel.cpp
        src/gui/progress_dialog.cpp
    )

    target_include_directories(eda_gui PRIVATE
        ${CMAKE_SOURCE_DIR}/include/gui
        ${CMAKE_SOURCE_DIR}/include/core
        ${CMAKE_SOURCE_DIR}/include/parser
        ${CMAKE_SOURCE_DIR}/include/placer
        ${CMAKE_SOURCE_DIR}/include/visualizer
    )

    target_link_libraries(eda_gui PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        eda_core
        eda_parser
        eda_placer
        eda_visualizer
        eda_utils
    )

    set_target_properties(eda_gui PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
    )
endif()

# CLI tool
add_executable(eda_cli src/main_cli.cpp)
target_link_libraries(eda_cli PRIVATE
    eda_core
    eda_parser
    eda_placer
    eda_visualizer
    eda_utils
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Install
install(TARGETS eda_cli
    RUNTIME DESTINATION bin
)

if(BUILD_GUI)
    install(TARGETS eda_gui
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION .
    )
endif()
