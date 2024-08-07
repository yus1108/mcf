#pragma once
#include <crtdbg.h>
#include <string>
#include <functional>

#include <parser/includes/lexer.h>

#if defined(_DEBUG)
#define FATAL_ASSERT(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); return false; } ((void)0)
#define FATAL_ERROR(FORMAT, ...) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); return false; } ((void)0)
#else
#define FATAL_ASSERT(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); return false; } ((void)0)
#define FATAL_ERROR(FORMAT, ...) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); return false; } ((void)0)
#endif

namespace UnitTest
{
	const mcf::Token::Data token_invalid = { mcf::Token::Type::INVALID, "invalid" };
	const mcf::Token::Data token_const = { mcf::Token::Type::KEYWORD_CONST, "const" };
	const mcf::Token::Data token_void = { mcf::Token::Type::KEYWORD_VOID, "void" };
	const mcf::Token::Data token_int8 = { mcf::Token::Type::KEYWORD_INT8, "int8" };
	const mcf::Token::Data token_int16 = { mcf::Token::Type::KEYWORD_INT16, "int16" };
	const mcf::Token::Data token_int32 = { mcf::Token::Type::KEYWORD_INT32, "int32" };
	const mcf::Token::Data token_int64 = { mcf::Token::Type::KEYWORD_INT64, "int64" };
	const mcf::Token::Data token_uint8 = { mcf::Token::Type::KEYWORD_UINT8, "uint8" };
	const mcf::Token::Data token_uint16 = { mcf::Token::Type::KEYWORD_UINT16, "uint16" };
	const mcf::Token::Data token_uint32 = { mcf::Token::Type::KEYWORD_UINT32, "uint32" };
	const mcf::Token::Data token_uint64 = { mcf::Token::Type::KEYWORD_UINT64, "uint64" };
	const mcf::Token::Data token_utf8 = { mcf::Token::Type::KEYWORD_UTF8, "utf8" };
	const mcf::Token::Data token_unused = { mcf::Token::Type::KEYWORD_UNUSED, "unused" };
	const mcf::Token::Data token_in = { mcf::Token::Type::KEYWORD_IN, "in" };

	class BaseTest
	{
	public:
		virtual inline ~BaseTest(void) noexcept {}
		virtual const bool Test(void) const noexcept = 0;
	};

	class Lexer final : BaseTest
	{
	public:
		explicit Lexer(void) noexcept;
		virtual const bool Test(void) const noexcept override final;

	private:
		std::vector<std::string> _names;
		std::vector<std::function<bool()>> _tests;
	};

	inline static void detect_memory_leak( long line = -1 )
	{
		//Also need this for memory leak code stuff
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetBreakAlloc(line); //Important!
	}
}
