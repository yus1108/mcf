#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

// ASSERT
#if defined(_DEBUG)
#define debug_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf(FORMAT, __VA_ARGS__); __debugbreak(); } ((void)0)
#else
#define debug_assert(PREDICATE, FORMAT, ...)
#endif

#if defined(_DEBUG)
#define default_break(FORMAT, ...) printf(FORMAT, __VA_ARGS__); __debugbreak(); break
#else
#define default_break(FORMAT, ...) break
#endif

// ARRAY
template<typename T>
constexpr const size_t array_type_size(T array[])
{
	array;
	return sizeof(T);
}

#define array_size(array) sizeof(array) / array_type_size(array);

