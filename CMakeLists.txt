cmake_minimum_required(VERSION 3.28)
project(Budget_Gameboy_Emulator)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)

if (MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)

option(USE_DX11_ON_WINDOWS "Use directX11 graphics api on windows" OFF)

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

# imgui library using sdl3 as platform backend
set(IMGUI_SOURCE_FILES
	vendor/imgui/imgui.cpp
	vendor/imgui/imgui_demo.cpp
	vendor/imgui/imgui_draw.cpp
	vendor/imgui/imgui_tables.cpp
	vendor/imgui/imgui_widgets.cpp
	vendor/imgui/backends/imgui_impl_sdl3.cpp
	vendor/imgui/misc/cpp/imgui_stdlib.cpp
)

if (WIN32 AND USE_DX11_ON_WINDOWS)
	# directX11 renderer backend for imgui
	list(APPEND IMGUI_SOURCE_FILES
		vendor/imgui/backends/imgui_impl_dx11.cpp
	)
else()
	# opengl3 renderer backend for imgui
	list(APPEND IMGUI_SOURCE_FILES
		vendor/imgui/backends/imgui_impl_opengl3.cpp
	)
endif()

add_library(Imgui STATIC
	${IMGUI_SOURCE_FILES}
)

target_include_directories(Imgui PRIVATE
	vendor/imgui/
	vendor/imgui/backends/
	vendor/SDL/include/
)

target_compile_features(Imgui PRIVATE cxx_std_17)

# BudgetGB executable

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
	src/renderers/renderer.h
	src/utils/file.cpp
	src/utils/file.h
	src/utils/vec.h
	src/opcodeLogger.cpp
	src/opcodeLogger.h
	src/ppu.cpp
	src/ppu.h
	src/dmgBootrom.cpp
	src/dmgBootrom.h
	src/joypad.cpp
	src/joypad.h
	src/config.cpp
	src/config.h
	src/emulatorConstants.h
	src/patternTileView.cpp
	src/patternTileView.h

	src/mappers/mapper.cpp
	src/mappers/mapper.h
	src/mappers/noMBC.cpp
	src/mappers/noMBC.h
	src/mappers/MBC1.cpp
	src/mappers/MBC1.h
	src/mappers/MBC2.cpp
	src/mappers/MBC2.h
	src/mappers/MBC3.cpp
	src/mappers/MBC3.h
)

if (WIN32 AND USE_DX11_ON_WINDOWS)
	list(APPEND SOURCE_FILES
		src/renderers/directX11/directX11.cpp
	)
else()
	list(APPEND SOURCE_FILES
		src/renderers/opengl/opengl.cpp
		src/renderers/opengl/shader.cpp
		src/renderers/opengl/shader.h
	)
endif()

add_executable(BudgetGB ${SOURCE_FILES})
target_compile_features(BudgetGB PRIVATE cxx_std_17)

if (APPLE)
	target_compile_definitions(BudgetGB PRIVATE USE_GL_VERSION_410)
endif(APPLE)

target_include_directories(BudgetGB PRIVATE
	src/
	src/renderers/

	vendor/fmt/include/
	vendor/nlohmann/include/
	vendor/glad/include/
	vendor/imgui/
	vendor/imgui/backends/
)

set(BudgetGbLibs 
	Imgui
	SDL3::SDL3 
	NlohmannJson 
	fmt::fmt
)

if (WIN32 AND USE_DX11_ON_WINDOWS)
	message(STATUS "BudgetGB Graphics API: DirectX11")
	list(APPEND BudgetGbLibs d3d11.lib d3dcompiler.lib)
else()
	message(STATUS "BudgetGB Graphics API: OpenGL")
	list(APPEND BudgetGbLibs Glad)
endif()


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_link_libraries(BudgetGB PRIVATE ${BudgetGbLibs})
	target_compile_options(BudgetGB PRIVATE /W4 /MT$<$<CONFIG:Debug>:d>)
	message(STATUS "MSVC COMPILER")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")

	if (APPLE)
		target_link_libraries(BudgetGB PRIVATE ${BudgetGbLibs})
	else()
		target_link_libraries(BudgetGB PRIVATE ${BudgetGbLibs} -static)
	endif()

	target_compile_options(BudgetGB PRIVATE -O3 -Wall -Wextra -Wpedantic)
	message(STATUS "GNU COMPILER")
endif()
