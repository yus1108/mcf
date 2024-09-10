#pragma once
#include <iostream>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// ASSERT
#if defined(_DEBUG)
#define MCF_DEBUG_ASSERT(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); } ((void)0)
#else
#define MCF_DEBUG_ASSERT(PREDICATE, FORMAT, ...)
#endif

#if defined(_DEBUG)
#define MCF_DEBUG_BREAK(FORMAT, ...) printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak();
#else
#define MCF_DEBUG_BREAK(FORMAT, ...)
#endif

#if defined(_DEBUG)
#define MCF_DEBUG_TODO(FORMAT, ...) printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak();
#else
#define MCF_DEBUG_TODO(FORMAT, ...)
#endif

#if defined(_DEBUG)
#define MCF_DEBUG_MESSAGE(FORMAT, ...) printf(FORMAT, __VA_ARGS__); printf("\n")
#else
#define MCF_DEBUG_MESSAGE(FORMAT, ...)
#endif

#if defined(_DEBUG)
#define MCF_DEBUG_ASSERT_RETURN_BOOL(PREDICATE_TO_RETURN, FORMAT, ...) if ((PREDICATE_TO_RETURN) == false) { printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); return false; } return true
#else
#define MCF_DEBUG_ASSERT_RETURN_BOOL(PREDICATE_TO_RETURN, FORMAT, ...) return (PREDICATE_TO_RETURN)
#endif

#if defined(_DEBUG)
#define MCF_DEBUG_MESSAGE_RETURN_BOOL(PREDICATE_TO_RETURN, FORMAT, ...) if ((PREDICATE_TO_RETURN) == false) { printf(FORMAT, __VA_ARGS__); printf("\n"); return false; } return true
#else
#define MCF_DEBUG_MESSAGE_RETURN_BOOL(PREDICATE_TO_RETURN, FORMAT, ...) return (PREDICATE_TO_RETURN)
#endif

#if defined(_DEBUG)
#define MCF_EXECUTE_AND_DEBUG_ASSERT(PREDICATE_TO_EXECUTE, FORMAT, ...) if ((PREDICATE_TO_EXECUTE) == false) { printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); } ((void)0)
#else
#define MCF_EXECUTE_AND_DEBUG_ASSERT(PREDICATE_TO_EXECUTE, FORMAT, ...) PREDICATE_TO_EXECUTE
#endif

namespace mcf
{
	namespace Internal
	{
		constexpr static const bool IS_DIGIT(const char byte) noexcept
		{
			return ('0' <= byte && byte <= '9');
		}

		template <class... Variadic>
		inline static std::string ErrorMessage(const char* const format, Variadic&& ...args)
		{
			std::string message;

			int messageLength = snprintf(nullptr, 0, format, args...);
			message.resize(messageLength + 1);

			snprintf(message.data(), messageLength + 1, format, args...);

#if defined(_DEBUG)
			std::cout << message << std::endl;
			__debugbreak();
#endif
			return message;
		}

		namespace UInt64
		{
			inline static unsigned __int64 Pow(unsigned __int8 base, unsigned __int8 exponent)
			{
				unsigned __int64 result = 1;
				for (unsigned __int8 i = 0; i < exponent; i++)
				{
					result *= static_cast<unsigned __int64>(base);
				}
				return result;
			}
		}
	}
}