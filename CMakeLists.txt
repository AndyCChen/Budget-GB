cmake_minimum_required(VERSION 3.28)
project(Budget_Gameboy_Emulator)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)

# nlhomannJson parsing library
add_library(NlohmannJson INTERFACE)
target_include_directories(NlohmannJson INTERFACE 
	vendor/nlohmann/includes/nlohmann/
)

# fmt library
add_subdirectory(vendor/fmt/)

# sdl3 library
add_subdirectory(vendor/SDL/ EXCLUDE_FROM_ALL)

# opengl
find_package(OpenGL REQUIRED)

# glad loader
add_library(glad STATIC
	vendor/glad/src/glad.c
	vendor/glad/include/glad/glad.h
	vendor/glad/include/KHR/khrplatform.h
)
target_include_directories(glad PRIVATE
	vendor/glad/include/
)

set(SOURCE_FILES
	main.cpp
	src/BudgetGB.cpp
	src/BudgetGB.h
	src/sm83.cpp
	src/sm83.h
	src/bus.cpp
	src/bus.h
	src/sm83JsonTest.cpp
	src/sm83JsonTest.h
	src/cartridge.cpp
	src/cartridge.h
	src/disassembler.cpp
	src/disassembler.h
	src/renderers/include/renderer.h
)

list(APPEND SOURCE_FILES
	src/renderers/opengl.cpp
)

add_executable(BudgetGB ${SOURCE_FILES})

target_include_directories(BudgetGB PRIVATE
	vendor/fmt/include/
	vendor/nlohmann/include/
	vendor/glad/include/
	src/renderers/include/
	src/
)

target_link_libraries(BudgetGB PRIVATE OpenGL::GL SDL3::SDL3 glad NlohmannJson fmt::fmt)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(BudgetGB PRIVATE /W4)
	message(STATUS "MSVC COMPILER")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	target_compile_options(BudgetGB PRIVATE -O3 -Wall -Wextra -Wpedantic)
	message(STATUS "GNU COMPILER")
endif()
