cmake_minimum_required(VERSION 3.28)
project(Budget_Gameboy_Emulator)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# nlhomannJson parsing library
add_library(NlohmannJson INTERFACE)
target_include_directories(NlohmannJson INTERFACE 
	vendor/nlohmann
)

# fmt library
add_subdirectory(vendor/fmt)

add_executable(BudgetGB
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
)

target_include_directories(BudgetGB PRIVATE
	vendor
	src
)

target_link_libraries(BudgetGB PRIVATE NlohmannJson fmt::fmt)

if (MSVC)
	target_compile_options(BudgetGB PRIVATE /W4)
else(MSVC)
	target_compile_options(BudgetGB PRIVATE -O3 -Wall -Wextra -Wpedantic)
endif(MSVC)
