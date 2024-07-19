// parser.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "common.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"

constexpr const char* PARSING_FAIL_MESSAGE_FORMAT = "(Line: %zu)(%zu)\n[Description]: ";
// ASSERT
#if defined(_DEBUG)
#define parsing_fail_assert(PREDICATE, ERROR_ID, TOKEN, FORMAT, ...) if ((PREDICATE) == false) \
{ \
	std::string pfa_message; \
	int pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer == nullptr) { return false; } \
	snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_message += "\n"; \
	mcf::parser::error pfa_error = { ERROR_ID, _lexer.get_name(), pfa_message, TOKEN.Line, TOKEN.Index }; \
	_errors.push(pfa_error); \
	__debugbreak(); \
	return false;\
} ((void)0)
#define parsing_fail_message(ERROR_ID, TOKEN, FORMAT, ...) \
{ \
	std::string pfa_message; \
	int pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer != nullptr) \
	{ \
	snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_message += "\n"; \
	} \
	mcf::parser::error pfa_error = { ERROR_ID, _lexer.get_name(), pfa_message, TOKEN.Line, TOKEN.Index }; \
	_errors.push( pfa_error ); \
	__debugbreak(); \
} ((void)0)
#else
#define parsing_fail_assert(PREDICATE, ERROR_ID, TOKEN, FORMAT, ...) if ((PREDICATE) == false) \
{ \
	std::string pfa_message; \
	int pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer == nullptr) { return false; } \
	snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_message += "\n"; \
	mcf::parser::error pfa_error = { ERROR_ID, _lexer.get_name(), pfa_message, TOKEN.Line, TOKEN.Index }; \
	_errors.push(pfa_error); \
	return false;\
} ((void)0)
#define parsing_fail_message(ERROR_ID, TOKEN, FORMAT, ...) \
{ \
	std::string pfa_message; \
	int pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer != nullptr) \
	{ \
	snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_message += "\n"; \
	} \
	mcf::parser::error pfa_error = { ERROR_ID, _lexer.get_name(), pfa_message, TOKEN.Line, TOKEN.Index }; \
	_errors.push( pfa_error ); \
} ((void)0)
#endif

namespace mcf
{
	namespace internal
	{
		static std::stack<mcf::parser::error> PARSER_ERRORS;

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
			"keyword_identifier_end",

			"custom_keyword_start",
			"custom_enum_type",
			"custom_enum_value",
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
		constexpr const size_t TOKEN_TYPES_SIZE = sizeof(TOKEN_TYPES) / mcf::array_type_size(TOKEN_TYPES);
		static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPES_SIZE, "token_type count is changed. this VARIABLE need to be changed as well");
	}
}

mcf::parser::parser(const std::string& input, const bool isFile) noexcept
	: _lexer(input, isFile)
{
	if (check_last_lexer_error() == false)
	{
		return;
	}
	read_next_token(); // _currentToken = invalid; _nextToken = valid;
	read_next_token(); // _currentToken = valid; _nextToken = valid;
}

const size_t mcf::parser::get_error_count(void) noexcept
{
	return _errors.size();
}

const mcf::parser::error mcf::parser::get_last_error(void) noexcept
{
	if (_errors.empty())
	{
		return { parser::error::id::no_error, _lexer.get_name(), std::string(), 0, 0};
	}

	const mcf::parser::error error = _errors.top();
	_errors.pop();
	return error;
}

void mcf::parser::parse_program(ast::program& outProgram) noexcept
{
	size_t capacity = 100;
	ast::statement_array statements = std::make_unique<ast::unique_statement[]>(capacity);
	size_t count = 0;
	while (_currentToken.Type != mcf::token_type::eof)
	{
		if (count >= capacity)
		{
			capacity *= 2;
			ast::statement_array temp = std::make_unique<ast::unique_statement[]>(capacity);
			for (size_t i = 0; i < count; i++)
			{
				temp[i] = ast::unique_statement(statements[i].release());
			}
			statements.reset(temp.release());
		}
		statements[count] = std::unique_ptr<const mcf::ast::statement>(parse_statement());
		if (statements[count++].get() == nullptr)
		{
			parsing_fail_message(error::id::fail_statement_parsing, _nextToken, u8"파싱에 실패하였습니다.");
		}

		// read next token
		read_next_token();
	}
	outProgram = ast::program(statements.release(), count);
}

inline const mcf::ast::statement* mcf::parser::parse_statement(void) noexcept
{
	std::unique_ptr<const ast::statement> statement;
	constexpr const size_t TOKEN_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (_currentToken.Type)
	{
	// !<declaration> 관련 키워드인 경우
	case token_type::keyword_const: __COUNTER__;
	case token_type::keyword_void: __COUNTER__;
	case token_type::keyword_int8: __COUNTER__;
	case token_type::keyword_int16: __COUNTER__;
	case token_type::keyword_int32: __COUNTER__;
	case token_type::keyword_int64: __COUNTER__;
	case token_type::keyword_uint8: __COUNTER__;
	case token_type::keyword_uint16: __COUNTER__;
	case token_type::keyword_uint32: __COUNTER__;
	case token_type::keyword_uint64: __COUNTER__;
	case token_type::keyword_utf8: __COUNTER__;
	case token_type::custom_enum_type: __COUNTER__;
		statement = std::unique_ptr<const ast::statement>(parse_variable_declaration_statement());
		break;

	case token_type::identifier: __COUNTER__;
		statement = std::unique_ptr<const ast::statement>(parse_variable_assignment_statement());
		break;

	case token_type::keyword_enum: __COUNTER__;
		statement = std::unique_ptr<const ast::statement>(parse_enum_statement());
		break;

	case token_type::macro_iibrary_file_include: __COUNTER__;
		parsing_fail_message(error::id::not_registered_statement_token, _currentToken, u8"#21 구현에 필요한 expressions & statements 개발");
		break;

	case token_type::macro_project_file_include: __COUNTER__;
		parsing_fail_message(error::id::not_registered_statement_token, _currentToken, u8"#21 구현에 필요한 expressions & statements 개발");
		break;

	case token_type::eof: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
		break;

	case token_type::integer: __COUNTER__;
	case token_type::string_utf8: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::plus: __COUNTER__;
	case token_type::minus: __COUNTER__;
	case token_type::asterisk: __COUNTER__;
	case token_type::slash: __COUNTER__;
	case token_type::lt: __COUNTER__;
	case token_type::gt: __COUNTER__;
	case token_type::ampersand: __COUNTER__;
	case token_type::lparen: __COUNTER__;
	case token_type::rparen: __COUNTER__;
	case token_type::lbrace: __COUNTER__;
	case token_type::rbrace: __COUNTER__;
	case token_type::lbracket: __COUNTER__;
	case token_type::rbracket: __COUNTER__;
	case token_type::colon: __COUNTER__;
	case token_type::comma: __COUNTER__;
	case token_type::keyword_identifier_start: __COUNTER__;
	case token_type::keyword_unused: __COUNTER__;
	case token_type::keyword_identifier_end: __COUNTER__;
	case token_type::custom_keyword_start: __COUNTER__;
	case token_type::custom_enum_value: __COUNTER__;
	case token_type::custom_keyword_end: __COUNTER__;
	case token_type::keyword_variadic: __COUNTER__;
	case token_type::macro_start: __COUNTER__;
	case token_type::macro_end: __COUNTER__;
	case token_type::comment: __COUNTER__;			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case token_type::comment_block: __COUNTER__;	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
		parsing_fail_message(error::id::not_registered_statement_token, _currentToken, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu) literal=`%s`",
			internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());
		break;
	}
	constexpr const size_t TOKEN_TYPE_COUNT = __COUNTER__ - TOKEN_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPE_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");
	return statement.release();
}

const mcf::ast::variable_statement* mcf::parser::parse_variable_declaration_statement(void) noexcept
{
	debug_assert(is_token_data_type(_currentToken) == true, u8"이 함수가 호출될때 현재 토큰은 데이터 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());
	const mcf::ast::data_type_expression dataType = parse_data_type_expressions();

	if (read_next_token_if(token_type::identifier) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `identifier`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}
	const mcf::ast::identifier_expression name(_currentToken);

	// optional
	std::unique_ptr<const mcf::ast::expression> rightExpression;
	if (read_next_token_if(token_type::assign) == true)
	{
		read_next_token();
		rightExpression = std::unique_ptr<const mcf::ast::expression>(parse_expression(mcf::parser::precedence::lowest));
		if (rightExpression == nullptr)
		{
			parsing_fail_message(error::id::fail_expression_parsing, _nextToken, u8"파싱에 실패하였습니다.");
			return nullptr;
		}
	}

	if (read_next_token_if(token_type::semicolon) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `semicolon`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}

	return new(std::nothrow) mcf::ast::variable_statement(dataType, name, rightExpression.release());
}

const mcf::ast::variable_assign_statement* mcf::parser::parse_variable_assignment_statement(void) noexcept
{
	debug_assert(_currentToken.Type == token_type::identifier, u8"이 함수가 호출될때 현재 토큰은 식별자 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	mcf::ast::identifier_expression	name(_currentToken);

	if (read_next_token_if(token_type::assign) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `assign`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}
	read_next_token();
	std::unique_ptr<const mcf::ast::expression> rightExpression = std::unique_ptr<const mcf::ast::expression>(parse_expression(mcf::parser::precedence::lowest));

	if (read_next_token_if(token_type::semicolon) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `semicolon`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}

	return new(std::nothrow) mcf::ast::variable_assign_statement(name, rightExpression.release());
}

const mcf::ast::enum_statement* mcf::parser::parse_enum_statement(void) noexcept
{
	debug_assert(_currentToken.Type == token_type::keyword_enum, u8"이 함수가 호출될때 현재 enum 키워드여야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	if (read_next_token_if(token_type::identifier) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `identifier`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}

	// 열거형 타입을 등록하고 데이터 타입으로 받는다.
	register_custom_enum_type(_currentToken);
	const ast::data_type_expression& name = parse_data_type_expressions();

	bool isUseDefaultDataType = true;
	if (read_next_token_if(token_type::colon) == true)
	{
		if (is_token_data_type(_nextToken) == false)
		{
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 데이터 타입이어야 합니다. 실제 값으로 %s를 받았습니다.",
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
			return nullptr;
		}
		read_next_token();
		isUseDefaultDataType = false;
	}
	const ast::data_type_expression& dataType = isUseDefaultDataType ? ast::data_type_expression(false, token{token_type::keyword_int32, "int32"}) : parse_data_type_expressions();

	if (read_next_token_if(token_type::lbrace) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `lbrace`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}

	std::unique_ptr<const ast::enum_block_statements_expression> values(parse_enum_block_statements_expression());
	if (values.get() == nullptr)
	{
		return nullptr;
	}

	if (read_next_token_if(token_type::rbrace) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `rbrace`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}

	if (read_next_token_if(token_type::semicolon) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `semicolon`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}

	return new(std::nothrow) ast::enum_statement(name, dataType, values.release());
}

const mcf::ast::expression* mcf::parser::parse_expression(const mcf::parser::precedence precedence) noexcept
{
	std::unique_ptr<const ast::expression> expression;
	constexpr const size_t EXPRESSION_TOKEN_COUNT_BEGIN = __COUNTER__;
	switch (_currentToken.Type)
	{
	case token_type::identifier: __COUNTER__;
		expression = std::make_unique<ast::identifier_expression>(_currentToken);
		break;
	
	case token_type::integer: __COUNTER__;
	case token_type::string_utf8: __COUNTER__;
		expression = std::make_unique<ast::literal_expession>(_currentToken);
		break;

	case token_type::plus: __COUNTER__;
	case token_type::minus: __COUNTER__;
		expression = std::unique_ptr<const ast::expression>(parse_prefix_expression());
		break;

	case token_type::keyword_const: __COUNTER__;
	case token_type::keyword_void: __COUNTER__;
	case token_type::keyword_int8: __COUNTER__;
	case token_type::keyword_int16: __COUNTER__;
	case token_type::keyword_int32: __COUNTER__;
	case token_type::keyword_int64: __COUNTER__;
	case token_type::keyword_uint8: __COUNTER__;
	case token_type::keyword_uint16: __COUNTER__;
	case token_type::keyword_uint32: __COUNTER__;
	case token_type::keyword_uint64: __COUNTER__;
	case token_type::keyword_utf8: __COUNTER__;
	case token_type::custom_enum_type: __COUNTER__;
		expression = std::make_unique<ast::data_type_expression>(parse_data_type_expressions());
		break;

	case token_type::keyword_unused: __COUNTER__;
		parsing_fail_message(error::id::not_registered_expression_token, _currentToken, u8"#21 구현에 필요한 expressions & statements 개발");
		break;

	case token_type::keyword_variadic: __COUNTER__;
		parsing_fail_message(error::id::not_registered_expression_token, _currentToken, u8"#21 구현에 필요한 expressions & statements 개발");
		break;

	case token_type::lparen: __COUNTER__;
		parsing_fail_message(error::id::not_registered_expression_token, _currentToken, u8"TODO: parse_block_expression 구현");
		break;

	case token_type::eof: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::asterisk: __COUNTER__;
	case token_type::slash: __COUNTER__;
	case token_type::lt: __COUNTER__;
	case token_type::gt: __COUNTER__;
	case token_type::ampersand: __COUNTER__;
	case token_type::rparen: __COUNTER__;
	case token_type::lbrace: __COUNTER__;
	case token_type::rbrace: __COUNTER__;
	case token_type::lbracket: __COUNTER__;
	case token_type::rbracket: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
	case token_type::colon: __COUNTER__;
	case token_type::comma: __COUNTER__;
	case token_type::keyword_identifier_start: __COUNTER__;
	case token_type::keyword_enum: __COUNTER__;
	case token_type::keyword_identifier_end: __COUNTER__;
	case token_type::custom_keyword_start: __COUNTER__;
	case token_type::custom_enum_value: __COUNTER__;
	case token_type::custom_keyword_end: __COUNTER__;
	case token_type::macro_start: __COUNTER__;
	case token_type::macro_iibrary_file_include: __COUNTER__;
	case token_type::macro_project_file_include: __COUNTER__;
	case token_type::macro_end: __COUNTER__;
	case token_type::comment: __COUNTER__;			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case token_type::comment_block: __COUNTER__;	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
		parsing_fail_message(error::id::not_registered_expression_token, _currentToken, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu)",
			internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type));
		break;
	}
	constexpr const size_t EXPRESSION_TOKEN_COUNT = __COUNTER__ - EXPRESSION_TOKEN_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::token_type::count) == EXPRESSION_TOKEN_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");

	while (expression.get() != nullptr && _nextToken.Type != token_type::semicolon && precedence < get_next_infix_expression_token_precedence())
	{
		constexpr const size_t INFIX_COUNT_BEGIN = __COUNTER__;
		switch (_nextToken.Type)
		{
		case token_type::plus: __COUNTER__;
		case token_type::minus: __COUNTER__;
		case token_type::asterisk: __COUNTER__;
		case token_type::slash: __COUNTER__;
		case token_type::lt: __COUNTER__;
		case token_type::gt: __COUNTER__;
			read_next_token();
			expression = std::unique_ptr<const ast::expression>(parse_infix_expression(expression.release()));
			break;

		case token_type::lparen: __COUNTER__;
			read_next_token();
			expression = std::unique_ptr<const ast::expression>(parse_call_expression(expression.release()));
			break;

		case token_type::lbracket: __COUNTER__;
			read_next_token();
			expression = std::unique_ptr<const ast::expression>(parse_index_expression(expression.release()));
			break;

		case token_type::ampersand: __COUNTER__;
			parsing_fail_message(error::id::not_registered_expression_token, _currentToken, u8"TODO 비트 연산자 파싱");
			break;

		case token_type::eof: __COUNTER__;
		case token_type::identifier: __COUNTER__;
		case token_type::integer: __COUNTER__;
		case token_type::string_utf8: __COUNTER__;
		case token_type::assign: __COUNTER__;
		case token_type::rparen: __COUNTER__;
		case token_type::lbrace: __COUNTER__;
		case token_type::rbrace: __COUNTER__;
		case token_type::rbracket: __COUNTER__;
		case token_type::semicolon: __COUNTER__;
		case token_type::colon: __COUNTER__;
		case token_type::comma: __COUNTER__;
		case token_type::keyword_identifier_start: __COUNTER__;
		case token_type::keyword_const: __COUNTER__;
		case token_type::keyword_void: __COUNTER__;
		case token_type::keyword_int8: __COUNTER__;
		case token_type::keyword_int16: __COUNTER__;
		case token_type::keyword_int32: __COUNTER__;
		case token_type::keyword_int64: __COUNTER__;
		case token_type::keyword_uint8: __COUNTER__;
		case token_type::keyword_uint16: __COUNTER__;
		case token_type::keyword_uint32: __COUNTER__;
		case token_type::keyword_uint64: __COUNTER__;
		case token_type::keyword_utf8: __COUNTER__;
		case token_type::keyword_enum: __COUNTER__;
		case token_type::keyword_unused: __COUNTER__;
		case token_type::keyword_identifier_end: __COUNTER__;
		case token_type::keyword_variadic: __COUNTER__;
		case token_type::custom_keyword_start: __COUNTER__;
		case token_type::custom_enum_type: __COUNTER__;
		case token_type::custom_enum_value: __COUNTER__;
		case token_type::custom_keyword_end: __COUNTER__;
		case token_type::macro_start: __COUNTER__;
		case token_type::macro_iibrary_file_include: __COUNTER__;
		case token_type::macro_project_file_include: __COUNTER__;
		case token_type::macro_end: __COUNTER__;
		case token_type::comment: __COUNTER__;			// 주석은 파서에서 토큰을 읽으면 안됩니다.
		case token_type::comment_block: __COUNTER__;	// 주석은 파서에서 토큰을 읽으면 안됩니다.
		default:
			parsing_fail_message(error::id::not_registered_infix_expression_token, _currentToken, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu)",
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)], enum_index(_nextToken.Type));
			return nullptr;
		}
		constexpr const size_t INFIX_COUNT = __COUNTER__ - INFIX_COUNT_BEGIN;
		static_assert(static_cast<size_t>(mcf::token_type::count) == INFIX_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");
	}

	return expression.release();
}

const mcf::ast::data_type_expression mcf::parser::parse_data_type_expressions(void) noexcept
{
	debug_assert(is_token_data_type(_currentToken) == true, u8"이 함수가 호출될때 현재 토큰은 데이터 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	const bool isConst = _currentToken.Type == token_type::keyword_const;
	if (isConst)
	{
		read_next_token();
	}
	debug_assert(is_token_data_type(_currentToken) == true && _currentToken.Type != token_type::keyword_const, 
		u8"이 함수가 호출될때 현재 토큰은 const 키워드를 제외한 데이터 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());
	return mcf::ast::data_type_expression(isConst, _currentToken);
}

const mcf::ast::prefix_expression* mcf::parser::parse_prefix_expression(void) noexcept
{
	const token prefixToken = _currentToken;

	read_next_token();

	std::unique_ptr<const ast::expression> targetExpression(parse_expression(precedence::prefix));
	if (targetExpression == nullptr)
	{
		return nullptr;
	}

	return new(std::nothrow) ast::prefix_expression(prefixToken, targetExpression.release());
}

const mcf::ast::infix_expression* mcf::parser::parse_infix_expression(const mcf::ast::expression* left) noexcept
{
	std::unique_ptr<const ast::expression> leftExpression(left);

	const token infixOperatorToken = _currentToken;
	const precedence precedence = get_current_infix_expression_token_precedence();
	read_next_token();
	std::unique_ptr<const ast::expression> rightExpression(parse_expression(precedence));
	if (rightExpression.get() == nullptr)
	{
		parsing_fail_message(error::id::fail_expression_parsing, _nextToken, u8"파싱에 실패하였습니다.");
		return nullptr;
	}
	return new(std::nothrow) ast::infix_expression(leftExpression.release(), infixOperatorToken, rightExpression.release());
}

const mcf::ast::infix_expression* mcf::parser::parse_call_expression(const mcf::ast::expression* left) noexcept
{
	unused(left);
	parsing_fail_message(error::id::not_registered_infix_expression_token, token(), u8"#18 기본적인 평가기 개발 구현 필요");
	return nullptr;
}

const mcf::ast::index_expression* mcf::parser::parse_index_expression(const mcf::ast::expression* left) noexcept
{
	debug_assert(_currentToken.Type == token_type::lbracket, u8"이 함수가 호출될때 현재 토큰은 'lbracket' 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	std::unique_ptr<const ast::expression> leftExpression(left);

	read_next_token();
	if (_currentToken.Type == token_type::lbracket)
	{
		read_next_token();
		return new(std::nothrow) ast::index_expression(leftExpression.release(), new(std::nothrow) ast::unknown_index_expression());
	}

	const token infixOperatorToken = _currentToken;
	const precedence precedence = get_current_infix_expression_token_precedence();
	read_next_token();
	std::unique_ptr<const ast::expression> rightExpression(parse_expression(precedence));
	if (rightExpression.get() == nullptr)
	{
		parsing_fail_message(error::id::fail_expression_parsing, _nextToken, u8"파싱에 실패하였습니다.");
		return nullptr;
	}
	return new(std::nothrow) ast::index_expression(leftExpression.release(), rightExpression.release());
}

const mcf::ast::enum_block_statements_expression* mcf::parser::parse_enum_block_statements_expression(void) noexcept
{
	using unique_expression = std::unique_ptr <const mcf::ast::expression>;
	using name_vector = std::vector<mcf::ast::identifier_expression>;
	using expression_array = std::unique_ptr<unique_expression[]>;

	debug_assert(_currentToken.Type == token_type::lbrace, u8"이 함수가 호출될때 현재 토큰은 'lbrace' 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	name_vector	names;
	size_t capacity = 100;
	size_t size = 0;
	expression_array values = std::make_unique<unique_expression[]>(capacity);

	while (read_next_token_if(token_type::identifier) == true)
	{
		names.emplace_back(_currentToken);

		// assign 타입이 들어오지 않으면 enum_value_increment 를 넣고 평가 단계에서 그전값에 1을 더한다.
		if (read_next_token_if(token_type::assign))
		{
			read_next_token();
			if (size >= capacity)
			{
				capacity *= 2;
				expression_array temp = std::make_unique<unique_expression[]>(capacity);
				for (size_t i = 0; i < size; i++)
				{
					temp[i] = unique_expression(values[i].release());
				}
				values.reset(temp.release());
			}
			values[size] = std::unique_ptr<const mcf::ast::expression>(parse_expression(mcf::parser::precedence::lowest));
			if (values[size++].get() == nullptr)
			{
				parsing_fail_message(error::id::fail_expression_parsing, _nextToken, u8"파싱에 실패하였습니다.");
				return nullptr;
			}
		}
		else
		{
			if (size >= capacity)
			{
				capacity *= 2;
				expression_array temp = std::make_unique<unique_expression[]>(capacity);
				for (size_t i = 0; i < size; i++)
				{
					temp[i] = unique_expression(values[i].release());
				}
				values.reset(temp.release());
			}
			values[size] = std::unique_ptr<const mcf::ast::expression>(new(std::nothrow) mcf::ast::enum_value_increment());
			if (values[size++].get() == nullptr)
			{
				parsing_fail_message(error::id::fail_expression_parsing, _nextToken, u8"파싱에 실패하였습니다.");
				return nullptr;
			}
		}

		// comma 가 오면 루프를 다시 돈다.
		if (read_next_token_if(token_type::comma))
		{
			continue;
		}

		break;
	}

	debug_assert(_nextToken.Type == token_type::rbrace, u8"이 함수가 끝날때 현재 토큰은 'rbrace' 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	return new(std::nothrow) mcf::ast::enum_block_statements_expression(names, values.release());
}

inline void mcf::parser::read_next_token(void) noexcept
{
	_currentToken = _nextToken;
	_nextToken = _lexer.read_next_token();
	// 읽은 토큰이 주석이라면 주석이 아닐때까지 읽는다.
	while (_nextToken.Type == token_type::comment || _nextToken.Type == token_type::comment_block)
	{
		_nextToken = _lexer.read_next_token();
	}
}

inline const bool mcf::parser::read_next_token_if(mcf::token_type tokenType) noexcept
{
	if (_nextToken.Type != tokenType)
	{
		return false;
	}
	read_next_token();
	return true;
}

inline const bool mcf::parser::is_token_data_type(const mcf::token& token) const noexcept
{
	constexpr const size_t TOKEN_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (token.Type)
	{
	// !<declaration> 관련 키워드인 경우
	case token_type::keyword_const: __COUNTER__;
	case token_type::keyword_void: __COUNTER__;
	case token_type::keyword_int8: __COUNTER__;
	case token_type::keyword_int16: __COUNTER__;
	case token_type::keyword_int32: __COUNTER__;
	case token_type::keyword_int64: __COUNTER__;
	case token_type::keyword_uint8: __COUNTER__;
	case token_type::keyword_uint16: __COUNTER__;
	case token_type::keyword_uint32: __COUNTER__;
	case token_type::keyword_uint64: __COUNTER__;
	case token_type::keyword_utf8: __COUNTER__;
	case token_type::custom_enum_type: __COUNTER__;
		return true;

	case token_type::eof: __COUNTER__;
	case token_type::identifier: __COUNTER__;
	case token_type::integer: __COUNTER__;
	case token_type::string_utf8: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::plus: __COUNTER__;
	case token_type::minus: __COUNTER__;
	case token_type::asterisk: __COUNTER__;
	case token_type::slash: __COUNTER__;
	case token_type::lt: __COUNTER__;
	case token_type::gt: __COUNTER__;
	case token_type::ampersand: __COUNTER__;
	case token_type::lparen: __COUNTER__;
	case token_type::rparen: __COUNTER__;
	case token_type::lbrace: __COUNTER__;
	case token_type::rbrace: __COUNTER__;
	case token_type::lbracket: __COUNTER__;
	case token_type::rbracket: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
	case token_type::colon: __COUNTER__;
	case token_type::comma: __COUNTER__;
	case token_type::keyword_identifier_start: __COUNTER__;
	case token_type::keyword_enum: __COUNTER__;
	case token_type::keyword_unused: __COUNTER__;
	case token_type::keyword_identifier_end: __COUNTER__;
	case token_type::keyword_variadic: __COUNTER__;
	case token_type::custom_keyword_start: __COUNTER__;
	case token_type::custom_enum_value: __COUNTER__;
	case token_type::custom_keyword_end: __COUNTER__;
	case token_type::macro_start: __COUNTER__;
	case token_type::macro_iibrary_file_include: __COUNTER__;
	case token_type::macro_project_file_include: __COUNTER__;
	case token_type::macro_end: __COUNTER__;
	case token_type::comment: __COUNTER__;			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case token_type::comment_block: __COUNTER__;	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
		break;
	}
	constexpr const size_t TOKEN_TYPE_COUNT = __COUNTER__ - TOKEN_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPE_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");
	return false;
}

inline const mcf::parser::precedence mcf::parser::get_infix_expression_token_precedence(const mcf::token& token) noexcept
{
	constexpr const size_t PRECEDENCE_COUNT_BEGIN = __COUNTER__;
	switch (token.Type)
	{
	case token_type::plus: __COUNTER__;
	case token_type::minus: __COUNTER__;
		return parser::precedence::sum;

	case token_type::asterisk: __COUNTER__;
	case token_type::slash: __COUNTER__;
		return parser::precedence::product;

	case token_type::lt: __COUNTER__;
	case token_type::gt: __COUNTER__;
		return parser::precedence::lessgreater;

	case token_type::lparen: __COUNTER__;
		return parser::precedence::call;

	case token_type::lbracket: __COUNTER__;
		return parser::precedence::index;

	case token_type::eof: __COUNTER__;
	case token_type::identifier: __COUNTER__;
	case token_type::integer: __COUNTER__;
	case token_type::string_utf8: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::ampersand: __COUNTER__;
	case token_type::rparen: __COUNTER__;
	case token_type::lbrace: __COUNTER__;
	case token_type::rbrace: __COUNTER__;
	case token_type::rbracket: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
	case token_type::colon: __COUNTER__;
	case token_type::comma: __COUNTER__;
	case token_type::keyword_identifier_start: __COUNTER__;
	case token_type::keyword_const: __COUNTER__;
	case token_type::keyword_void: __COUNTER__;
	case token_type::keyword_int8: __COUNTER__;
	case token_type::keyword_int16: __COUNTER__;
	case token_type::keyword_int32: __COUNTER__;
	case token_type::keyword_int64: __COUNTER__;
	case token_type::keyword_uint8: __COUNTER__;
	case token_type::keyword_uint16: __COUNTER__;
	case token_type::keyword_uint32: __COUNTER__;
	case token_type::keyword_uint64: __COUNTER__;
	case token_type::keyword_utf8: __COUNTER__;
	case token_type::keyword_enum: __COUNTER__;
	case token_type::keyword_unused: __COUNTER__;
	case token_type::keyword_identifier_end: __COUNTER__;
	case token_type::keyword_variadic: __COUNTER__;
	case token_type::custom_keyword_start: __COUNTER__;
	case token_type::custom_enum_type: __COUNTER__;
	case token_type::custom_enum_value: __COUNTER__;
	case token_type::custom_keyword_end: __COUNTER__;
	case token_type::macro_start: __COUNTER__;
	case token_type::macro_iibrary_file_include: __COUNTER__;
	case token_type::macro_project_file_include: __COUNTER__;
	case token_type::macro_end: __COUNTER__;
	case token_type::comment: __COUNTER__;			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case token_type::comment_block: __COUNTER__;	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
		parsing_fail_message(parser::error::id::not_registered_infix_expression_token, token, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu)",
			internal::TOKEN_TYPES[enum_index(token.Type)], enum_index(token.Type));
		break;
	}
	constexpr const size_t PRECEDENCE_COUNT = __COUNTER__ - PRECEDENCE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(token_type::count) == PRECEDENCE_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");

	return parser::precedence::lowest;
}

inline const mcf::parser::precedence mcf::parser::get_next_infix_expression_token_precedence(void) noexcept
{
	return get_infix_expression_token_precedence(_nextToken);
}

inline const mcf::parser::precedence mcf::parser::get_current_infix_expression_token_precedence(void) noexcept
{
	return get_infix_expression_token_precedence(_currentToken);
}

const bool mcf::parser::check_last_lexer_error(void) noexcept
{
	// 렉서에 에러가 있는지 체크
	lexer::error_token lexerError = _lexer.get_last_error_token();
	constexpr const size_t LEXER_ERROR_TOKEN_COUNT_BEGIN = __COUNTER__;
	switch (lexerError)
	{
	case lexer::error_token::no_error: __COUNTER__;
		return true;

	case lexer::error_token::invalid_input_length: __COUNTER__;
		parsing_fail_message(error::id::invalid_input_length, token(), u8"input의 길이가 0입니다.");
		break;

	case lexer::error_token::fail_read_file: __COUNTER__;
		parsing_fail_message(error::id::fail_read_file, token(), u8"파일 읽기에 실패 하였습니다. file path=%s", _lexer.get_name().c_str());
		break;

	case lexer::error_token::fail_memory_allocation: __COUNTER__;
		parsing_fail_message(error::id::fail_memory_allocation, token(), u8"메모리 할당에 실패 하였습니다. file path=%s", _lexer.get_name().c_str());
		break;

	case lexer::error_token::registering_duplicated_symbol_name: __COUNTER__;
		parsing_fail_message(error::id::registering_duplicated_symbol_name, token(), u8"심볼이 중복되는 타입이 등록 되었습니다. file path=%s", _lexer.get_name().c_str());
		break;

	default:
		parsing_fail_message(error::id::invalid_lexer_error_token, token(), u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. parser::error::id=invalid_lexer_error_token");
		break;
	}
	constexpr const size_t LEXER_ERROR_TOKEN_COUNT = __COUNTER__ - LEXER_ERROR_TOKEN_COUNT_BEGIN;
	static_assert(static_cast<size_t>(lexer::error_token::count) == LEXER_ERROR_TOKEN_COUNT, "lexer error_token is changed. this SWITCH need to be changed as well.");

	check_last_lexer_error();
	return false;
}

const bool mcf::parser::register_custom_enum_type(mcf::token& inOutToken) noexcept
{
	debug_assert(inOutToken.Type == token_type::identifier, "identifier 타입의 토큰만 커스텀 타입으로 변경 가능합니다.");
	inOutToken.Type = _lexer.register_custom_enum_type(inOutToken.Literal);
	parsing_fail_assert(inOutToken.Type == token_type::custom_enum_type, error::id::registering_duplicated_symbol_name, inOutToken, 
		"심볼이 중복되는 타입이 등록 되었습니다. 타입=%s", inOutToken.Literal.c_str());
	return true;
}
