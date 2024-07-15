#pragma once

#define unused(variable) variable
#define array_size(value) sizeof(value) / mcf::array_type_size(value)

namespace mcf
{
	// ENUM
	template<typename T>
	constexpr const size_t enum_count(void)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum type is required for this function");
		return static_cast<size_t>(T::count);
	}

	template<typename T>
	constexpr const size_t enum_count(const T value)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum value is required for this function");
		return static_cast<size_t>(T::count);
	}

	template<typename T>
	constexpr const size_t enum_index(const T value)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum value is required for this function");
		return value < T::count ? static_cast<size_t>(value) : static_cast<size_t>(T::invalid);
	}

	template<typename T>
	const T enum_at(const size_t index)
	{
		static_assert(std::is_enum_v<T> == true, u8"only enum value is required for this function");
		return index < mcf::enum_count<T>() ? static_cast<T>(index) : T::invalid;
	}

	// ARRAY
	template<typename T>
	constexpr const size_t array_type_size(T array[])
	{
		array;
		return sizeof(T);
	}
}