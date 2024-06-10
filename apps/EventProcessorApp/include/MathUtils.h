#pragma once

#include <cstddef> // Include the header file for size_t definition

// Function to check if a given integer is a power of two
constexpr bool is_power_of_two(int v) noexcept
{
    return v && ((v & (v - 1)) == 0); // Bitwise AND of v and (v-1) checks if it's a power of two
}

// Function to find the next power of two greater than or equal to a given size_t
constexpr size_t next_power_of_two(size_t n) noexcept
{
    n--; // Decrement n to handle edge cases where n is already a power of two
    // Bitwise OR with right-shifted values creates a mask with all bits set to 1 from the most significant bit to the least significant set bit
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    if (sizeof(size_t) > 4) {
        n |= n >> 32; // For 64-bit systems, extend the mask further
    }
    ++n; // Increment n to get the next power of two
    return n;
}
