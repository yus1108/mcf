#pragma once
#include <iostream>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// ASSERT
#if defined(_DEBUG)
#define DebugAssert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); } ((void)0)
#else
#define DebugAssert(PREDICATE, FORMAT, ...)
#endif

#if defined(_DEBUG)
#define DebugBreak(FORMAT, ...) printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); break
#else
#define DebugBreak(FORMAT, ...) break
#endif

#if defined(_DEBUG)
#define DebugMessage(FORMAT, ...) printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak();
#else
#define DebugMessage(FORMAT, ...)
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