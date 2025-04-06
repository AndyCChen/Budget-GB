cmake_minimum_required(VERSION 3.28)
project(Budget_Gameboy_Emulator)

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

# glad loader
add_library(Glad STATIC
	vendor/glad/src/glad.c
)

target_include_directories(Glad PRIVATE
	vendor/glad/include/
)

# imgui library
set(IMGUI_SOURCE_FILES
	vendor/imgui/imgui.cpp
	vendor/imgui/imgui_demo.cpp
	vendor/imgui/imgui_draw.cpp
	vendor/imgui/imgui_tables.cpp
	vendor/imgui/imgui_widgets.cpp
)

# sdl3 + opengl3 backends for imgui
list(APPEND IMGUI_SOURCE_FILES
	vendor/imgui/backends/imgui_impl_sdl3.cpp
	vendor/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(Imgui STATIC
	${IMGUI_SOURCE_FILES}
)

target_include_directories(Imgui PRIVATE
	vendor/imgui/
	vendor/imgui/backends/
	vendor/SDL/include/
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
target_compile_features(BudgetGB PRIVATE cxx_std_17)

target_include_directories(BudgetGB PRIVATE
	src/
	src/renderers/include/

	vendor/fmt/include/
	vendor/nlohmann/include/
	vendor/glad/include/
	vendor/imgui/
	vendor/imgui/backends/
)

target_link_libraries(BudgetGB PRIVATE 
	Imgui
	SDL3::SDL3 
	Glad 
	NlohmannJson 
	fmt::fmt
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(BudgetGB PRIVATE /W4)
	message(STATUS "MSVC COMPILER")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	target_compile_options(BudgetGB PRIVATE -O3 -Wall -Wextra -Wpedantic)
	message(STATUS "GNU COMPILER")
endif()
