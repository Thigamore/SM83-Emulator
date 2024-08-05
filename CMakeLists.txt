cmake_minimum_required(VERSION 3.12)

set(SRC ${SM83_SOURCE_DIR}/src/cpu.h ${SM83_SOURCE_DIR}/src/cpu.cpp ${SM83_SOURCE_DIR}/src/debug.cpp ${SM83_SOURCE_DIR}/src/debug.h ${SM83_SOURCE_DIR}/src/helper.cpp ${SM83_SOURCE_DIR}/src/helper.h ${SM83_SOURCE_DIR}/src/memory.h ${SM83_SOURCE_DIR}/src/memory.cpp)

project(
    SM83 
    VERSION 0.1
    DESCRIPTION "An emulator of an SM83"
    LANGUAGES CXX
)

add_executable(main src/main.cpp ${SRC})

enable_testing()
add_subdirectory(tests)