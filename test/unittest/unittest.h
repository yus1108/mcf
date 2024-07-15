#pragma once
#include <stdlib.h>
#include <crtdbg.h>
#include <Windows.h>
#include <string>
#include <functional>

#include <parser/includes/common.h>
#include <parser/includes/ast.h>
#include <parser/includes/lexer.h>
#include <parser/includes/parser.h>

#if defined(_DEBUG)
#define fatal_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); return false; } ((void)0)
#define fatal_error(FORMAT, ...) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); return false; } ((void)0)
#else
#define fatal_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); return false; } ((void)0)
#define fatal_error(FORMAT, ...) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); return false; } ((void)0)
#endif

#define error_message_begin(ERROR_COUNT) printf(u8"[Error]: %s(Line: %d) %zu개의 에러 메시지가 있습니다.\n", ##__FILE__, ##__LINE__, ERROR_COUNT)
#define error_message(FORMAT, ...) printf(FORMAT, __VA_ARGS__); printf("\n"); ((void)0)
#if defined(_DEBUG)
#define error_message_end __debugbreak(); abort();
#else
#define error_message_end abort();
#endif

namespace UnitTest
{
	constexpr const char* TOKEN_TYPES[] =
	{
		"invalid",
		"eof",

		// 식별자 + 리터럴
		"identifier",
		"integer",

		// 연산자
		"assign",
		"plus",
		"minus",
		"asterisk",
		"slash",
		"lt",
		"gt",

		"lparen",
		"rparen",
		"lbrace",
		"rbrace",
		"lbracket",
		"rbracket",


		// 구분자
		"semicolon",
		"comma",

		// 예약어
		"keyword",
	};
	constexpr const size_t TOKEN_TYPES_SIZE = array_size(TOKEN_TYPES);
	static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPES_SIZE,
		"token count not matching!");

	constexpr const char* STATEMENT_TYPES[] =
	{
		"invalid",

		"variable_declaration",
		"variable_assignment",
	};
	constexpr const size_t STATEMENT_TYPES_SIZE = array_size(STATEMENT_TYPES);
	static_assert(static_cast<size_t>(mcf::ast::statement_type::count) == STATEMENT_TYPES_SIZE,
		"statement_type count not matching");

	constexpr const char* EXPRESSION_TYPES[] =
	{
		"invalid",

		"literal",
		"identifier",
		"data_type",
		"prefix",
		"infix",
	};
	constexpr const size_t EXPRESSION_TYPES_SIZE = array_size(EXPRESSION_TYPES);
	static_assert(static_cast<size_t>(mcf::ast::expression_type::count) == EXPRESSION_TYPES_SIZE,
		"expression_type count not matching");

	const std::string convert_to_string(const mcf::token& token);

	class UnitTest
	{
	public:
		virtual inline ~UnitTest(void) noexcept {}
		virtual const bool Test(void) const noexcept = 0;
	};

	class Lexer final : UnitTest
	{
	public:
		explicit Lexer(void) noexcept;
		virtual const bool Test(void) const noexcept override final;

	private:
		std::vector<std::string>			_names;
		std::vector<std::function<bool()>>	_tests;
	};

	class Parser final : UnitTest
	{
	public:
		explicit Parser(void) noexcept;
		virtual const bool Test(void) const noexcept override final;

	private:
		static void check_parser_errors(mcf::parser& parser) noexcept;
		static bool test_variable_declaration_statement(const mcf::ast::statement* statement, const mcf::token_type expectedDataType, const std::string& expectedName) noexcept;
		static bool test_literal(const mcf::ast::expression* expression, const mcf::token& expectedToken) noexcept;
		static bool test_identifier(const mcf::ast::expression* targetExpression, const mcf::token& expectedToken) noexcept;

	private:
		std::vector<std::string>			_names;
		std::vector<std::function<bool()>>	_tests;
	};

	inline static void detect_memory_leak(long line = -1)
	{
		//Also need this for memory leak code stuff
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetBreakAlloc(line); //Important!
	}
}

