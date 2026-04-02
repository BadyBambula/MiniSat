cmake_minimum_required(VERSION 3.10.0)
project(MiniSat VERSION 0.1.0 LANGUAGES C CXX)

add_executable(solver
	main.cpp
	cnf/cnf_parser.cpp
    solver/sat_solver.cpp
)

target_include_directories(solver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

