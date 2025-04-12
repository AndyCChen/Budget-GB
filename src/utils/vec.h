#pragma once

#include <array>
#include <cstdint>

namespace Utils
{
typedef std::array<uint8_t, 3> array_u8Vec3; // vec3 of unsigned 8-bit ints as a array

// vec2 as a struct
template <typename T> 
struct struct_Vec2
{
	T x = 0;
	T y = 0;
};

} // namespace Utils
