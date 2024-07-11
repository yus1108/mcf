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

// ENUM
#define ENUM_COUNT(enum) static_cast<size_t>(##enum##::count)

template<typename T>
constexpr const size_t ENUM_INDEX(const T value)
{
	static_assert(std::is_enum_v<T> == true, u8"해당 함수는 enum value만 사용 가능합니다");
	return static_cast<size_t>(value);
}

template<typename T>
const T ENUM_AT(const size_t index)
{
	static_assert(std::is_enum_v<T> == true, u8"해당 함수는 enum value만 사용 가능합니다.");
	debug_assert(index < ENUM_COUNT(T), u8"index가 해당 enum의 크기보다 큽니다. index=%zu", index);
	return static_cast<T>(index);
}

// ARRAY
template<typename T>
constexpr const size_t ARRAY_TYPE_SIZE(T array[])
{
	array;
	return sizeof(T);
}

#define ARRAY_SIZE(array) sizeof(array) / ARRAY_TYPE_SIZE(array);

