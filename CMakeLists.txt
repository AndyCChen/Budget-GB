cmake_minimum_required(VERSION 3.28)
project(Budget_Gameboy_Emulator)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_library(NlohmannJson INTERFACE)
target_include_directories(NlohmannJson INTERFACE 
	libs/nlohmann
)

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
)

target_include_directories(BudgetGB PRIVATE
	libs
	src
)

target_link_libraries(BudgetGB PRIVATE NlohmannJson)

if (MSVC)
	target_compile_options(BudgetGB PRIVATE /W4 /Zc:__cplusplus)
else(MSVC)
	target_compile_options(BudgetGB PRIVATE -Wall -Wextra -Wpedantic)
endif(MSVC)