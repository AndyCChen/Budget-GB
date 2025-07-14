#pragma once

#include <array>
#include <cstdint>

namespace BudgetGbConstants
{
static constexpr uint32_t LCD_WIDTH    = 160;
static constexpr uint32_t LCD_HEIGHT   = 144;
static constexpr uint32_t CLOCK_RATE_T = 4194304; // gameboy clock frequency

typedef std::array<uint8_t, LCD_WIDTH * LCD_HEIGHT> LcdColorBuffer;
} // namespace BudgetGbConstants
