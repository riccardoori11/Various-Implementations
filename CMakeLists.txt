cmake_minimum_required(VERSION 3.21)

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

# Namespaced name for projects that use this library (e.g. the compiler):
#   target_link_libraries(my_target PRIVATE ricc::implementation)
add_library(ricc::implementation ALIAS implementation)

# Everything below (demo executables, Catch2, tests) is only for working on this
# repo itself. When another project pulls this one in with FetchContent or
# add_subdirectory, PROJECT_IS_TOP_LEVEL is false and it only gets the library.
if(PROJECT_IS_TOP_LEVEL)



add_executable(vector
  vector.cpp 

)

add_executable(hive
  hive.cpp 

)

add_executable(string
  string.cpp 

)

add_executable(unique_ptr
  unique_ptr.cpp 

)

add_executable(any
  any.cpp 

)

add_executable( forward_list
list.cpp

)

add_executable( function
function.cpp

)

add_executable(variant
  variant.cpp 

)

add_executable(quicksort
  quicksort.cpp 

)

add_executable(dequeue
  dequeue.cpp 

)

add_executable(shared_ptr
  shared_ptr.cpp 

)

add_executable(dup_ptr
  dup_ptr.cpp 

)

add_executable(benchmark
  benchmark.cpp 

)

add_executable( lock_based_stack
lock_based_stack.cpp
)

add_executable( thread_safe_queue
thread_safe_queue.cpp
)

add_executable( accumulate
accumulate.cpp
)

add_executable( array
array.cpp
)


target_compile_options(benchmark PRIVATE
    $<$<CONFIG:Release>:-O3>
)

target_link_libraries(dequeue
    PRIVATE
        implementation
)

target_link_libraries(any
    PRIVATE
        implementation
)

target_link_libraries(unique_ptr
    PRIVATE
        implementation
)
target_link_libraries(benchmark
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

target_link_libraries(variant
    PRIVATE
        implementation
)

target_link_libraries(function
    PRIVATE
        implementation
)

target_link_libraries(forward_list
    PRIVATE
        implementation
)

target_link_libraries(dup_ptr
    PRIVATE
        implementation
)

target_link_libraries(shared_ptr
    PRIVATE
        implementation
)

target_link_libraries(lock_based_stack
    PRIVATE
        implementation
)

target_link_libraries(quicksort
    PRIVATE
        implementation
)

target_link_libraries(array
    PRIVATE
        implementation
)

target_link_libraries(accumulate
    PRIVATE
        implementation
)

target_link_libraries( thread_safe_queue
    PRIVATE
        implementation
)


target_link_libraries( hive
    PRIVATE
        implementation
)

# target_compile_options(implementation INTERFACE
#-Wall
#-Wextra
#-Wpedantic
#-Wconversion
#$<$<CONFIG:Debug>:-g>
#$<$<CONFIG:Debug>:-fsanitize=address,undefined>
#$<$<CONFIG:Debug>:-fno-omit-frame-pointer>
#)

#target_link_options(implementation INTERFACE
#$<$<CONFIG:Debug>:-fsanitize=address,undefined>
#)



# ---------------------------------------------------------------------------
# Tests (Catch2) — built with warnings and sanitizers enabled
# ---------------------------------------------------------------------------
option(ENABLE_SANITIZERS "Build tests with AddressSanitizer + UBSan" ON)

include(FetchContent)
FetchContent_Declare(Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.7.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(tests
    tests/test_string.cpp
    tests/test_vector.cpp
)

target_link_libraries(tests
    PRIVATE
        implementation
        Catch2::Catch2WithMain
)

target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_SANITIZERS)
    target_compile_options(tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
    target_link_options(tests PRIVATE -fsanitize=address,undefined)
endif()

include(Catch)
catch_discover_tests(tests)

# unique_ptr tests live in their own executable so that, while unique_ptr is
# being extended, compile errors here don't block the Vector/String tests.
add_executable(unique_ptr_tests
    tests/test_unique_ptr.cpp
)

target_link_libraries(unique_ptr_tests
    PRIVATE
        implementation
        Catch2::Catch2WithMain
)

target_compile_options(unique_ptr_tests PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_SANITIZERS)
    target_compile_options(unique_ptr_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
    target_link_options(unique_ptr_tests PRIVATE -fsanitize=address,undefined)
endif()

catch_discover_tests(unique_ptr_tests)

# optional tests: own executable while optional.hpp is being written.
add_executable(optional_tests
    tests/test_optional.cpp
)

target_link_libraries(optional_tests
    PRIVATE
        implementation
        Catch2::Catch2WithMain
)

target_compile_options(optional_tests PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_SANITIZERS)
    target_compile_options(optional_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
    target_link_options(optional_tests PRIVATE -fsanitize=address,undefined)
endif()

catch_discover_tests(optional_tests)

# String features the compiler project needs: own executable while string.hpp
# is being extended, so its compile errors don't block the other tests.
add_executable(string_tests
    tests/test_string_compiler.cpp
)

target_link_libraries(string_tests
    PRIVATE
        implementation
        Catch2::Catch2WithMain
)

target_compile_options(string_tests PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_SANITIZERS)
    target_compile_options(string_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
    target_link_options(string_tests PRIVATE -fsanitize=address,undefined)
endif()

catch_discover_tests(string_tests)

endif() # PROJECT_IS_TOP_LEVEL
