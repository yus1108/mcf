#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#if defined(_DEBUG)
#define wdebug_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { wprintf(FORMAT, __VA_ARGS__); __debugbreak(); } ((void)0)
#else
#define wdebug_assert(PREDICATE, FORMAT, ...)
#endif

#if defined(_DEBUG)
#define wdefault_break(FORMAT, ...) wprintf(FORMAT, __VA_ARGS__); __debugbreak(); break
#else
#define wdefault_break(FORMAT, ...) break
#endif