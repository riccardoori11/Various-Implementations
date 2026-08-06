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


add_executable(vector
  vector.cpp 

)

add_executable(string
  string.cpp 

)

add_executable(unique_ptr
  unique_ptr.cpp 

)

target_link_libraries(unique_ptr
    PRIVATE
        implementation
)

target_link_libraries(string
    PRIVATE
        implementation
)

target_link_libraries(vector
    PRIVATE
        implementation
)


