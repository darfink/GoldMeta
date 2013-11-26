#pragma once

#include <boost/format.hpp>
#include <cstdint>

// This is used extensively, so we define it here
using boost::format;
using boost::str;

typedef uint8_t  byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t uint64;
typedef int64_t  int64;

// Some API functions expect an unsigned long
typedef unsigned long ulong;

// Since we use assembly in our project, we assume a specific size from each type
static_assert(sizeof(char)   == sizeof(int8_t),  "Incompatible size of 'char'");
static_assert(sizeof(short)  == sizeof(int16_t), "Incompatible size of 'short'");
static_assert(sizeof(int)    == sizeof(int32_t), "Incompatible size of 'int'");
static_assert(sizeof(float)  == 4,               "Incompatible size of 'float'");
static_assert(sizeof(double) == 8,               "Incompatible size of 'double'");

// Used to get the size of arrays with a static size
#define GM_ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

// Can always be useful to have
template <typename T> T clamp(const T& value, const T& low, const T& high) {
    return (value < low) ? low : (value > high ? high : value);
}
