#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// ENUM
#define ENUM_COUNT(enum) static_cast<size_t>(##enum##::count)

template<typename T>
constexpr const size_t ENUM_INDEX(const T value)
{
	static_assert(std::is_enum_v<T> == true, u8"해당 함수는 enum value만 사용 가능합니다");
	return static_cast<size_t>(value);
}

template<typename T>
constexpr const T ENUM_AT(const size_t index)
{
	static_assert(std::is_enum_v<T> == true, u8"해당 함수는 enum value만 사용 가능합니다.");
	static_assert(ENUM_COUNT(T) <= index, u8"index가 해당 enum의 크기보다 큽니다.");
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