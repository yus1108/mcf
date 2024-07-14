#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// ARRAY
template<typename T>
constexpr const size_t array_type_size(T array[])
{
	array;
	return sizeof(T);
}

#define array_size(array) sizeof(array) / array_type_size(array);