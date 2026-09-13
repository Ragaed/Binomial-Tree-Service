cmake_minimum_required(VERSION 3.16)

project(binomial_tree_service LANGUAGES CXX)

include(CTest)

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(rate_lattice src/rate_lattice.cpp src/backward_induction.cpp)
target_compile_features(rate_lattice PUBLIC cxx_std_17)
target_include_directories(rate_lattice PUBLIC src)

if(MSVC)
    target_compile_options(rate_lattice PRIVATE /W4 /WX)
else()
    target_compile_options(rate_lattice PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rate_lattice PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

add_executable(rate_lattice_test tests/rate_lattice_test.cpp)
target_link_libraries(rate_lattice_test PRIVATE rate_lattice)

if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rate_lattice_test PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(rate_lattice_test PRIVATE -fsanitize=address,undefined)
endif()

if(BUILD_TESTING)
    add_test(NAME rate_lattice_test COMMAND rate_lattice_test)
endif()