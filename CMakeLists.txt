cmake_minimum_required(VERSION 3.31)

project(implementation LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(implementation INTERFACE)

target_include_directories(implementation
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(implementation
    INTERFACE
        cxx_std_20
)

add_executable(app
    main.cpp
)

target_link_libraries(app
    PRIVATE
        implementation
)


