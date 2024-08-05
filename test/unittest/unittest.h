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
#include <parser/includes/evaluator.h>

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
		"integer",
		"string_utf8",

		// 연산자
		"assign",
		"plus",
		"minus",
		"asterisk",
		"slash",
		"bang",
		"equal",
		"not_equal",
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
		"double_colon",
		"semicolon",
		"comma",

		// 식별자 키워드
		"keyword_identifier_start",
		"keyword_const",
		"keyword_void",
		"keyword_int8",
		"keyword_int16",
		"keyword_int32",
		"keyword_int64",
		"keyword_uint8",
		"keyword_uint16",
		"keyword_uint32",
		"keyword_uint64",
		"keyword_utf8",
		"keyword_enum",
		"keyword_unused",
		"keyword_in",
		"keyword_out",
		"keyword_bool",
		"keyword_true",
		"keyword_false",
		"keyword_identifier_end",

		"custom_keyword_start",
		"custom_enum_type",
		"custom_keyword_end",

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
		"function",
		"function_call",
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
		"index_unknown",
		"index",
		"function_parameter",
		"function_parameter_variadic",
		"function_parameter_list",
		"function_block",
		"function_call",
		"enum_value_increment",
		"enum_block",
	};
	constexpr const size_t EXPRESSION_TYPES_SIZE = array_size( EXPRESSION_TYPES );
	static_assert(static_cast<size_t>(mcf::ast::expression_type::count) == EXPRESSION_TYPES_SIZE, "expression_type count not matching");

	const mcf::token token_invalid = { mcf::token_type::invalid, "invalid" };
	const mcf::token token_const = { mcf::token_type::keyword_const, "const" };
	const mcf::token token_void = { mcf::token_type::keyword_void, "void" };
	const mcf::token token_int8 = { mcf::token_type::keyword_int8, "int8" };
	const mcf::token token_int16 = { mcf::token_type::keyword_int16, "int16" };
	const mcf::token token_int32 = { mcf::token_type::keyword_int32, "int32" };
	const mcf::token token_int64 = { mcf::token_type::keyword_int64, "int64" };
	const mcf::token token_uint8 = { mcf::token_type::keyword_uint8, "uint8" };
	const mcf::token token_uint16 = { mcf::token_type::keyword_uint16, "uint16" };
	const mcf::token token_uint32 = { mcf::token_type::keyword_uint32, "uint32" };
	const mcf::token token_uint64 = { mcf::token_type::keyword_uint64, "uint64" };
	const mcf::token token_utf8 = { mcf::token_type::keyword_utf8, "utf8" };
	const mcf::token token_unused = { mcf::token_type::keyword_unused, "unused" };
	const mcf::token token_in = { mcf::token_type::keyword_in, "in" };


	const mcf::ast::data_type_expression type_int8(false, token_int8);
	const mcf::ast::data_type_expression type_int16(false, token_int16);
	const mcf::ast::data_type_expression type_int32(false, token_int32);
	const mcf::ast::data_type_expression type_int64(false, token_int64);
	const mcf::ast::data_type_expression type_uint8(false, token_uint8);
	const mcf::ast::data_type_expression type_uint16(false, token_uint16);
	const mcf::ast::data_type_expression type_uint32(false, token_uint32);
	const mcf::ast::data_type_expression type_uint64(false, token_uint64);

	const mcf::ast::data_type_expression type_const_int8(true, token_int8);
	const mcf::ast::data_type_expression type_const_int16(true, token_int16);
	const mcf::ast::data_type_expression type_const_int32(true, token_int32);
	const mcf::ast::data_type_expression type_const_int64(true, token_int64);
	const mcf::ast::data_type_expression type_const_uint8(true, token_uint8);
	const mcf::ast::data_type_expression type_const_uint16(true, token_uint16);
	const mcf::ast::data_type_expression type_const_uint32(true, token_uint32);
	const mcf::ast::data_type_expression type_const_uint64(true, token_uint64);
	const mcf::ast::data_type_expression type_const_utf8(true, token_utf8);

	inline const mcf::ast::identifier_expression Identifier(const char* const value)
	{
		return mcf::ast::identifier_expression({ mcf::token_type::identifier, value });
	}

	inline std::unique_ptr<const mcf::ast::identifier_expression> NewIdentifier(const char* const value)
	{
		return std::make_unique<mcf::ast::identifier_expression>(mcf::token{mcf::token_type::identifier, value});
	}

	inline std::unique_ptr<const mcf::ast::data_type_expression> NewDataType(bool isConst, mcf::token token)
	{
		return std::make_unique<mcf::ast::data_type_expression>(isConst, token);
	}

	inline std::unique_ptr<const mcf::ast::literal_expession> NewInt(int32_t value)
	{
		return std::make_unique<mcf::ast::literal_expession>(mcf::token{ mcf::token_type::integer, std::to_string(value) });
	}

	inline std::unique_ptr<const mcf::ast::literal_expession> NewString(const char* const value)
	{
		return std::make_unique<mcf::ast::literal_expession>(mcf::token{ mcf::token_type::string_utf8, std::string("\"") + value + "\"" });
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

		static bool check_parser_errors(mcf::parser& parser) noexcept;
	private:
		static bool test_variable_declaration_statement(const mcf::ast::statement* statement, const mcf::token_type expectedDataType, const std::string& expectedName) noexcept;
		static bool test_expression(const mcf::ast::expression* actual, const mcf::ast::expression* expected) noexcept;
		static bool test_literal(const mcf::ast::expression* expression, const mcf::token& expectedToken) noexcept;
		static bool test_identifier(const mcf::ast::expression* targetExpression, const std::string expectedValue) noexcept;

	private:
		std::vector<std::string>			_names;
		std::vector<std::function<bool()>>	_tests;
	};

	class Evaluator final : UnitTest
	{
	public:
		explicit Evaluator(void) noexcept;
		virtual const bool Test(void) const noexcept override final;

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
