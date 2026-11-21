
cmake_minimum_required(VERSION 3.20)

project(AspireLight
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AspireLight
    src/main.cpp
    src/hardware/HardwareDetector.cpp
)

target_include_directories(AspireLight PRIVATE include)

target_link_libraries(AspireLight PRIVATE setupapi advapi32)

if(MSVC)
    target_compile_options(AspireLight PRIVATE /W4 /EHsc)
endif()
