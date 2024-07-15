#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// ASSERT
#if defined(_DEBUG)
#define debug_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); } ((void)0)
#else
#define debug_assert(PREDICATE, FORMAT, ...)
#endif

#if defined(_DEBUG)
#define default_break(FORMAT, ...) printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); break
#else
#define default_break(FORMAT, ...) break
#endif