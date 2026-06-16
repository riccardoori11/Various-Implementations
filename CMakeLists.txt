cmake_minimum_required(VERSION 3.31)



project(implementation)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


target_include_directories(implementation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(implementation
    PUBLIC
        cxx_std_20
)

