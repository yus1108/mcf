#pragma once

#if defined(UNUSED)
#undef MCF_UNUSED
#endif
#define MCF_UNUSED(...) __VA_ARGS__

#if defined(MCF_ARRAY_SIZE)
#undef MCF_ARRAY_SIZE
#endif
#define MCF_ARRAY_SIZE(value) sizeof(value) / mcf::ARRAY_TYPE_SIZE(value)

namespace mcf
{
	// ENUM
	template<typename T>
	constexpr const size_t ENUM_COUNT(void)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum type is required for this function");
		return static_cast<size_t>(T::COUNT);
	}

	template<typename T>
	constexpr const size_t ENUM_COUNT(const T value)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum value is required for this function");
		return static_cast<size_t>(T::COUNT);
	}

	template<typename T>
	constexpr const size_t ENUM_INDEX(const T value)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum value is required for this function");
		return value < T::COUNT ? static_cast<size_t>(value) : static_cast<size_t>(T::INVALID);
	}

	template<typename T>
	constexpr const T ENUM_AT(const size_t index)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum value is required for this function");
		return index < mcf::ENUM_COUNT<T>() ? static_cast<T>(index) : T::INVALID;
	}

	// ARRAY
	template<typename T>
	constexpr const size_t ARRAY_TYPE_SIZE(T array[])
	{
		array;
		return sizeof(T);
	}
}