cmake_minimum_required(VERSION 3.16)

# Example executables
add_executable(example_bookshelf example_bookshelf.cpp)
target_link_libraries(example_bookshelf PRIVATE
    eda_core
    eda_parser
)

add_executable(example_verilog example_verilog.cpp)
target_link_libraries(example_verilog PRIVATE
    eda_core
    eda_parser
)

add_executable(example_circuit example_circuit.cpp)
target_link_libraries(example_circuit PRIVATE
    eda_core
)

add_executable(example_placement example_placement.cpp)
target_link_libraries(example_placement PRIVATE
    eda_core
    eda_parser
    eda_placer
    eda_visualizer
)
