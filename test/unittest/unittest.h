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

#define error_message_begin(ERROR_COUNT) printf(u8"[Error][Parser]: %s(Line: %d) %zu개의 에러 메시지가 있습니다.\n", ##__FILE__, ##__LINE__, ERROR_COUNT)
#define error_message(FORMAT, ...) printf(FORMAT, __VA_ARGS__); printf("\n"); ((void)0)
#if defined(_DEBUG)
#define error_message_end __debugbreak();
#else
#define error_message_end
#endif

namespace UnitTest
{
	constexpr const char* TOKEN_TYPES[] =
	{
		"invalid",
		"eof",

		// 식별자 + 리터럴
		"identifier",
		"numberic_literal_start",
		"integer_8bit",
		"integer_32bit",
		"unsigned_integer_8bit",
		"unsigned_integer_32bit",
		"numberic_literal_end",
		"string_utf8",

		// 연산자
		"assign",
		"plus",
		"minus",
		"asterisk",
		"slash",
		"lt",
		"gt",
		"ampersand",

		"lparen",
		"rparen",
		"lbrace",
		"rbrace",
		"lbracket",
		"rbracket",


		// 구분자
		"colon",
		"semicolon",
		"comma",

		// 식별자 키워드
		"keyword_identifier_start",
		"keyword_const",
		"keyword_void",
		"keyword_uint8",
		"keyword_uint32",
		"keyword_int8",
		"keyword_int32",
		"keyword_utf8",
		"keyword_enum",
		"keyword_unused",
		"keyword_identifier_end",

		// '.' 으로 시작하는 토큰
		"keyword_variadic",

		// 매크로
		"macro_start",
		"macro_iibrary_file_include",
		"macro_project_file_include",
		"macro_end",

		"comment",
		"comment_block",
	};
	constexpr const size_t TOKEN_TYPES_SIZE = array_size( TOKEN_TYPES );
	static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPES_SIZE, "token count not matching!");

	constexpr const char* STATEMENT_TYPES[] =
	{
		"invalid",

		"variable",
		"variable_assign",
		"enum_def",
	};
	constexpr const size_t STATEMENT_TYPES_SIZE = array_size( STATEMENT_TYPES );
	static_assert(static_cast<size_t>(mcf::ast::statement_type::count) == STATEMENT_TYPES_SIZE, "statement_type count not matching");

	constexpr const char* EXPRESSION_TYPES[] =
	{
		"invalid",

		"literal",
		"identifier",
		"data_type",
		"prefix",
		"infix",
		"enum_block",
		"enum_value_increment",
	};
	constexpr const size_t EXPRESSION_TYPES_SIZE = array_size( EXPRESSION_TYPES );
	static_assert(static_cast<size_t>(mcf::ast::expression_type::count) == EXPRESSION_TYPES_SIZE, "expression_type count not matching");

	constexpr const char* PARSER_ERROR_ID[] =
	{
		"invalid",

		"no_error",
		"invalid_lexer_error_token",
		"invalid_input_length",
		"fail_read_file",
		"fail_memory_allocation",
		"fail_expression_parsing",
		"fail_statement_parsing",
		"unexpected_next_token",
		"not_registered_statement_token",
		"not_registered_expression_token",
		"not_registered_infix_expression_token",
	};
	constexpr const size_t PARSER_ERROR_ID_SIZE = array_size(PARSER_ERROR_ID);
	static_assert(static_cast<size_t>(mcf::parser::error::id::count) == PARSER_ERROR_ID_SIZE, "mcf::parser::error::id count not matching");

	const mcf::token token_const = { mcf::token_type::keyword_const, "const" };
	const mcf::token token_void = { mcf::token_type::keyword_void, "void" };
	const mcf::token token_int8 = { mcf::token_type::keyword_int8, "int8" };
	const mcf::token token_int32 = { mcf::token_type::keyword_int32, "int32" };
	const mcf::token token_uint8 = { mcf::token_type::keyword_uint8, "uint8" };
	const mcf::token token_uint32 = { mcf::token_type::keyword_uint32, "uint32" };
	const mcf::token token_utf8 = { mcf::token_type::keyword_utf8, "utf8" };

	const mcf::ast::data_type_expression type_int8(false, token_int8);
	const mcf::ast::data_type_expression type_int32(false, token_int32);
	const mcf::ast::data_type_expression type_uint8(false, token_uint8);
	const mcf::ast::data_type_expression type_uint32(false, token_uint32);

	const mcf::ast::data_type_expression type_const_int8(true, token_int8);
	const mcf::ast::data_type_expression type_const_int32(true, token_int32);
	const mcf::ast::data_type_expression type_const_uint8(true, token_uint8);
	const mcf::ast::data_type_expression type_const_uint32(true, token_uint32);

	inline const mcf::ast::identifier_expression Identifier(const char* const value)
	{
		return mcf::ast::identifier_expression({ mcf::token_type::identifier, value });
	}

	const std::string convert_to_string( const mcf::token& token );

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
		static bool check_parser_errors(mcf::parser& parser) noexcept;
		static bool test_variable_declaration_statement(const mcf::ast::statement* statement, const mcf::token_type expectedDataType, const std::string& expectedName) noexcept;
		static bool test_literal(const mcf::ast::expression* expression, const mcf::token& expectedToken) noexcept;
		static bool test_identifier(const mcf::ast::expression* targetExpression, const mcf::token& expectedToken) noexcept;

	private:
		std::vector<std::string>			_names;
		std::vector<std::function<bool()>>	_tests;
	};

	inline static void detect_memory_leak( long line = -1 )
	{
		//Also need this for memory leak code stuff
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetBreakAlloc(line); //Important!
	}
}
