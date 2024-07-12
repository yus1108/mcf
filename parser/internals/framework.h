#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// ASSERT
#if defined(_DEBUG)
#define parsing_fail_assert(PREDICATE, ERROR_ID, FORMAT, ...) if ((PREDICATE) == false) \
{ \
	std::string pfa_message; \
	int pfa_bufferLength = snprintf(nullptr, 0, "Fatal Error: ID[%d]: %s(Line: %d)\n[Description]: ", ERROR_ID, ##__FILE__, ##__LINE__); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	snprintf(pfa_buffer, pfa_bufferLength + 1, "[Fatal Error: ID[%d]: %s(Line: %d)\n[Description]: ", ERROR_ID, ##__FILE__, ##__LINE__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
	pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_message += "\n"; \
	mcf::parser::error pfa_error = { ERROR_ID, pfa_message }; \
	mcf::internal::PARSER_ERRORS.push(pfa_error); \
	__debugbreak(); \
	return false;\
} ((void)0)
#else
#define parsing_fail_assert(PREDICATE, ERROR_ID, FORMAT, ...) if ((PREDICATE) == false)  \
{ \
	std::string pfa_message; \
	int pfa_bufferLength = snprintf(nullptr, 0, "Fatal Error: ID[%d]: %s(Line: %d)\n[Description]: ", ERROR_ID, ##__FILE__, ##__LINE__); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	snprintf(pfa_buffer, pfa_bufferLength + 1, "[Fatal Error: ID[%d]: %s(Line: %d)\n[Description]: ", ERROR_ID, ##__FILE__, ##__LINE__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
	pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_message += "\n"; \
	mcf::parser::error pfa_error = { ERROR_ID, pfa_message }; \
	mcf::internal::PARSER_ERRORS.push(pfa_error); \
	return false;\
} ((void)0)
#endif

// ENUM
#define enum_count(enum) static_cast<size_t>(##enum##::count)

template<typename T>
constexpr const size_t enum_index(const T value)
{
	static_assert(std::is_enum_v<T> == true, u8"해당 함수는 enum value만 사용 가능합니다");
	return static_cast<size_t>(value);
}

template<typename T>
constexpr const T enum_at(const size_t index)
{
	static_assert(std::is_enum_v<T> == true, u8"해당 함수는 enum value만 사용 가능합니다.");
	static_assert(enum_count(T) <= index, u8"index가 해당 enum의 크기보다 큽니다.");
	return static_cast<T>(index);
}

// ARRAY
template<typename T>
constexpr const size_t array_type_size(T array[])
{
	array;
	return sizeof(T);
}

#define array_size(array) sizeof(array) / array_type_size(array);