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
			"keyword_in",
			"keyword_out",
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
	: _scope(std::vector<std::string>({ "global" }))
	, _lexer(input, isFile)
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
	ast::statement_array statements;
	while (_currentToken.Type != mcf::token_type::eof)
	{
		statements.emplace_back(std::unique_ptr<const mcf::ast::statement>(parse_statement()));
		if (statements.back().get() == nullptr)
		{
			parsing_fail_message(error::id::fail_statement_parsing, _nextToken, u8"파싱에 실패하였습니다.");
		}

		// read next token
		read_next_token();
	}
	outProgram = ast::program(std::move(statements));
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
		statement = std::unique_ptr<const ast::statement>(parse_declaration_statement());
		break;

	case token_type::identifier: __COUNTER__;
		statement = std::unique_ptr<const ast::statement>(parse_call_or_assign_statement());
		break;

	case token_type::keyword_enum: __COUNTER__;
		statement = std::unique_ptr<const ast::statement>(parse_enum_statement());
		break;

	case token_type::macro_iibrary_file_include: __COUNTER__;
		statement = std::make_unique<const ast::macro_include_statement>(_currentToken);
		break;

	case token_type::macro_project_file_include: __COUNTER__;
		statement = std::make_unique<const ast::macro_include_statement>(_currentToken);
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
	case token_type::keyword_in: __COUNTER__;
	case token_type::keyword_out: __COUNTER__;
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

const mcf::ast::statement* mcf::parser::parse_declaration_statement(void) noexcept
{
	debug_assert(is_token_data_type(_currentToken) == true, u8"이 함수가 호출될때 현재 토큰은 데이터 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	ast::statement_type statementType = ast::statement_type::invalid;
	std::unique_ptr<const mcf::ast::expression> dataType(parse_data_type_expressions());

	// 배열 타입인지 체크 (함수만 가능)
	while (read_next_token_if(token_type::lbracket))
	{
		// 이곳에선 현재 함수인지 변수 인지 알 수 없기 때문에 타입 체크를 하지 않습니다.
		statementType = ast::statement_type::function;
		dataType.reset(parse_index_expression(dataType.release()));
	}

	if (read_next_token_if(token_type::identifier) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `identifier`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}
	std::unique_ptr<const ast::expression> name(new(std::nothrow) ast::identifier_expression(_currentToken));

	// 배열 타입인지 체크 (변수만 가능)
	while (read_next_token_if(token_type::lbracket))
	{
		if (statementType == ast::statement_type::function)
		{
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"현재 토큰은 statementType=`function`일때 들어올 수 없습니다. _currentToken=`%s`",
				internal::TOKEN_TYPES[enum_index(_currentToken.Type)]);
			return nullptr;
		}
		statementType = ast::statement_type::variable;
		name = ast::unique_expression(parse_index_expression(name.release()));

		if (_currentToken.Type != token_type::rbracket)
		{
			parsing_fail_message(error::id::unexpected_next_token, _currentToken, u8"현재 토큰은 `rbracket`여야만 합니다. 실제 값으로 %s를 받았습니다.",
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
			return nullptr;
		}
	}

	// 함수인지 체크 하고 맞는경우 함수문으로 파싱한다.
	if (read_next_token_if(token_type::lparen))
	{
		if (statementType == ast::statement_type::variable)
		{
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"현재 토큰은 statementType=`variable`일때 들어올 수 없습니다. _currentToken=`%s`",
				internal::TOKEN_TYPES[enum_index(_currentToken.Type)]);
			return nullptr;
		}
		statementType = ast::statement_type::function;

		ast::unique_function_parameter_list parameters(parse_function_parameters());

		if (read_next_token_if(token_type::rparen) == false)
		{
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `rparen`여야만 합니다. 실제 값으로 %s를 받았습니다.",
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
			return nullptr;
		}

		ast::unique_function_block statementsBlock;
		if (read_next_token_if(token_type::lbrace, name->convert_to_string()) == true)
		{
			statementsBlock.reset(parse_function_block_expression());

			_scope.pop_back();
			if (read_next_token_if(token_type::rbrace) == false)
			{
				parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `rbrace`여야만 합니다. 실제 값으로 %s를 받았습니다.",
					internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
				return nullptr;
			}
		}
		else if (read_next_token_if(token_type::semicolon) == false)
		{
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `lbrace` 또는 `semicolon`여야만 합니다. 실제 값으로 %s를 받았습니다.",
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
			return nullptr;
		}

		return new(std::nothrow) ast::function_statement(dataType.release(), *static_cast<const ast::identifier_expression*>(name.get()), parameters.release(), statementsBlock.release());
	}

	if (statementType == ast::statement_type::function)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"현재 토큰은 statementType=`function`일때 들어올 수 없습니다. _currentToken=`%s`",
			internal::TOKEN_TYPES[enum_index(_currentToken.Type)]);
		return nullptr;
	}
	statementType = ast::statement_type::variable;

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

	return new(std::nothrow) mcf::ast::variable_statement(*static_cast<const ast::data_type_expression*>(dataType.get()), name.release(), rightExpression.release());
}

const mcf::ast::statement* mcf::parser::parse_call_or_assign_statement(void) noexcept
{
	debug_assert(_currentToken.Type == token_type::identifier, u8"이 함수가 호출될때 현재 토큰은 식별자 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	ast::unique_expression name(new(std::nothrow) ast::identifier_expression(_currentToken));

	// 함수 호출인지 체크
	while (read_next_token_if(token_type::lparen))
	{
		name.reset(parse_call_expression(name.release()));
		if (_currentToken.Type != token_type::rparen)
		{
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `rparen`여야만 합니다. 실제 값으로 %s를 받았습니다.",
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
			return nullptr;
		}
	}

	if (name->get_expression_type() == ast::expression_type::function_call)
	{
		if (read_next_token_if(token_type::semicolon) == false)
		{
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `semicolon`여야만 합니다. 실제 값으로 %s를 받았습니다.",
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
			return nullptr;
		}
		return new(std::nothrow) ast::function_call_statement(static_cast<const ast::function_call_expression*>(name.release()));
	}

	// 배열 타입인지 체크 (함수만 가능)
	while (read_next_token_if(token_type::lbracket))
	{
		name.reset(parse_index_expression(name.release()));
	}

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

	return new(std::nothrow) mcf::ast::variable_assign_statement(name.release(), rightExpression.release());
}

const mcf::ast::enum_statement* mcf::parser::parse_enum_statement(void) noexcept
{
	debug_assert(_currentToken.Type == token_type::keyword_enum, u8"이 함수가 호출될때 현재 enum 키워드여야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	if (read_next_token_if(token_type::identifier) == false)
	{
		// 커스텀 데이터인 경우면 스코프에 따라 중복 등록이 가능하다.
		if (_nextToken.Type != token_type::custom_enum_type)
		{
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `identifier` 또는 커스텀 데이터 타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
			return nullptr;
		}
		
		if (_lexer.has_datatype_at(_nextToken.Literal, _scope) == true)
		{
			std::string scopeString;
			for (size_t i = 0; i < _scope.size(); i++)
			{
				scopeString = (i == 0 ? "" : "::") + _scope[i];
			}
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"같은 스코프안에 중복되는 타입이 있습니다.. 실제 값으로 %s::%s를 받았습니다.",
				scopeString.c_str(), _nextToken.Literal.c_str());
			return nullptr;
		}

		read_next_token();
	}

	// 열거형 타입을 등록하고 데이터 타입으로 받는다.
	register_custom_enum_type(_currentToken);
	const ast::data_type_expression name = *std::unique_ptr<const ast::data_type_expression>(parse_data_type_expressions()).get();

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

	ast::data_type_expression dataType = isUseDefaultDataType ? 
		ast::data_type_expression(false, token{ token_type::keyword_int32, "int32" }) : 
		*std::unique_ptr<const ast::data_type_expression>(parse_data_type_expressions()).get();

	if (read_next_token_if(token_type::lbrace, name.convert_to_string()) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `lbrace`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}

	std::unique_ptr<const ast::enum_block_expression> values(parse_enum_block_expression());
	if (values.get() == nullptr)
	{
		_scope.pop_back();
		return nullptr;
	}

	_scope.pop_back();
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
		expression = std::unique_ptr<const ast::data_type_expression>(parse_data_type_expressions());
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
	case token_type::keyword_unused: __COUNTER__;
	case token_type::keyword_in: __COUNTER__;
	case token_type::keyword_out: __COUNTER__;
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
		case token_type::keyword_in: __COUNTER__;
		case token_type::keyword_out: __COUNTER__;
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

const mcf::ast::data_type_expression* mcf::parser::parse_data_type_expressions(void) noexcept
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
	return new(std::nothrow) mcf::ast::data_type_expression(isConst, _currentToken);
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

const mcf::ast::function_call_expression* mcf::parser::parse_call_expression(const mcf::ast::expression* left) noexcept
{
	debug_assert(_currentToken.Type == token_type::lparen, u8"이 함수가 호출될때 현재 토큰은 'lparen' 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	ast::unique_expression function(left);
	ast::expression_array parameters;

	// 인수가 없는 경우 바로 리턴한다.
	if (read_next_token_if(token_type::rparen) == true)
	{
		return new(std::nothrow) ast::function_call_expression(function.release(), std::move(parameters));
	}

	read_next_token();
	parameters.emplace_back(parse_expression(precedence::lowest));
	if (parameters.back().get() == nullptr)
	{
		parsing_fail_message(error::id::fail_expression_parsing, _nextToken, u8"파싱에 실패하였습니다.");
		return nullptr;
	}

	while (read_next_token_if(token_type::comma))
	{
		read_next_token();
		parameters.emplace_back(parse_expression(precedence::lowest));
		if (parameters.back().get() == nullptr)
		{
			parsing_fail_message(error::id::fail_expression_parsing, _nextToken, u8"파싱에 실패하였습니다.");
			return nullptr;
		}
	}

	if (read_next_token_if(token_type::rparen) == false)
	{
		parsing_fail_message(error::id::unexpected_next_token, _currentToken, u8"현재 토큰은 `rparen`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}
	return new(std::nothrow) ast::function_call_expression(function.release(), std::move(parameters));
}

const mcf::ast::index_expression* mcf::parser::parse_index_expression(const mcf::ast::expression* left) noexcept
{
	debug_assert(_currentToken.Type == token_type::lbracket, u8"이 함수가 호출될때 현재 토큰은 'lbracket' 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	std::unique_ptr<const ast::expression> leftExpression(left);

	if (read_next_token_if(token_type::rbracket))
	{
		return new(std::nothrow) ast::index_expression(leftExpression.release(), new(std::nothrow) ast::unknown_index_expression());
	}

	read_next_token();
	std::unique_ptr<const ast::expression> rightExpression(parse_expression(precedence::lowest));
	if (rightExpression.get() == nullptr)
	{
		parsing_fail_message(error::id::fail_expression_parsing, _nextToken, u8"파싱에 실패하였습니다.");
		return nullptr;
	}

	if (_currentToken.Type != token_type::rbracket)
	{
		parsing_fail_message(error::id::unexpected_next_token, _currentToken, u8"현재 토큰은 `rbracket`여야만 합니다. 실제 값으로 %s를 받았습니다.",
			internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
		return nullptr;
	}
	return new(std::nothrow) ast::index_expression(leftExpression.release(), rightExpression.release());
}

const mcf::ast::function_parameter_list_expression* mcf::parser::parse_function_parameters(void) noexcept
{
	debug_assert(_currentToken.Type == token_type::lparen, u8"이 함수가 호출될때 현재 토큰은 `lparen` 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	// `rparen`이 바로 나온다면 바로 종료
	if (read_next_token_if(token_type::rparen))
	{
		return new(std::nothrow) ast::function_parameter_list_expression();
	}

	std::vector<ast::unique_expression> list;

	while (list.empty() || (list.empty() == false && read_next_token_if(token_type::comma)))
	{
		const bool hasForToken = read_next_token_if_any({ token_type::keyword_unused, token_type::keyword_in, token_type::keyword_out });
		const token dataFor = hasForToken ? _currentToken : token{token_type::invalid, "invalid"};

		// `unused|in|out <data_type> <identifier>` 혹은 `[optional: unused] keyword_variadic` 의 표현식을 가지고 있는지 체크
		if (hasForToken == true && read_next_token_if_any(get_data_type_list()))
		{
			ast::unique_expression dataType;
			ast::unique_expression dataName;

			dataType = ast::unique_expression(parse_data_type_expressions());

			if (read_next_token_if(token_type::identifier) == false)
			{
				parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 `identifier`여야만 합니다. 실제 값으로 %s를 받았습니다.",
					internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
				return nullptr;
			}
			dataName = ast::unique_expression(new(std::nothrow) ast::identifier_expression(_currentToken));

			// 배열 타입인지 체크
			while (read_next_token_if(token_type::lbracket))
			{
				dataName.reset(parse_index_expression(dataName.release()));
			}

			list.emplace_back(new(std::nothrow) ast::function_parameter_expression(dataFor, dataType.release(), dataName.release()));
		}
		else if (read_next_token_if({ token_type::keyword_variadic }))
		{
			list.emplace_back(new(std::nothrow) ast::function_parameter_variadic_expression(dataFor));
			// variadic 은 항상 parameter 의 끝에 있어야 합니다.
			break;
		}
		else
		{
			parsing_fail_message(error::id::unexpected_next_token, _nextToken, u8"앞의 표현식 형식은 `unused|in|out <data_type> <identifier>` 혹은 `[optional: unused] keyword_variadic`여야만 합니다.");
			return nullptr;
		}
	}

	debug_assert(_nextToken.Type == token_type::rparen, u8"이 함수가 끝날때 다음 토큰은 'rparen' 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	return new(std::nothrow) ast::function_parameter_list_expression(std::move(list));
}

const mcf::ast::function_block_expression* mcf::parser::parse_function_block_expression(void) noexcept
{
	debug_assert(_currentToken.Type == token_type::lbrace, u8"이 함수가 호출될때 현재 토큰은 `lbrace` 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	ast::statement_array statements;
	while (_nextToken.Type != token_type::rbrace)
	{
		read_next_token();

		if (_currentToken.Type == mcf::token_type::eof)
		{
			parsing_fail_message(error::id::unexpected_current_token, _nextToken, u8"파싱에 실패하였습니다.");
			return nullptr;
		}

		statements.emplace_back(std::unique_ptr<const mcf::ast::statement>(parse_statement()));
		if (statements.back().get() == nullptr)
		{
			parsing_fail_message(error::id::fail_memory_allocation, _nextToken, u8"파싱에 실패하였습니다.");
		}
	}

	debug_assert(_nextToken.Type == token_type::rbrace, u8"이 함수가 호출될때 다음 토큰은 `rbrace` 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	return new(std::nothrow) ast::function_block_expression(std::move(statements));
}

const mcf::ast::enum_block_expression* mcf::parser::parse_enum_block_expression(void) noexcept
{
	using name_vector = std::vector<mcf::ast::identifier_expression>;

	debug_assert(_currentToken.Type == token_type::lbrace, u8"이 함수가 호출될때 현재 토큰은 'lbrace' 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	name_vector	names;
	ast::expression_array values;

	while (read_next_token_if(token_type::identifier) == true)
	{
		names.emplace_back(_currentToken);

		// assign 타입이 들어오지 않으면 enum_value_increment 를 넣고 평가 단계에서 그전값에 1을 더한다.
		if (read_next_token_if(token_type::assign))
		{
			read_next_token();
			values.emplace_back(parse_expression(mcf::parser::precedence::lowest));
			if (values.back().get() == nullptr)
			{
				parsing_fail_message(error::id::fail_expression_parsing, _nextToken, u8"파싱에 실패하였습니다.");
				return nullptr;
			}
		}
		else
		{
			values.emplace_back(new(std::nothrow) mcf::ast::enum_value_increment());
			if (values.back().get() == nullptr)
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

	debug_assert(_nextToken.Type == token_type::rbrace, u8"이 함수가 끝날때 다음 토큰은 'rbrace' 타입이어야만 합니다! 현재 token_type=%s(%zu) literal=`%s`",
		internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());

	return new(std::nothrow) mcf::ast::enum_block_expression(names, std::move(values));
}

inline void mcf::parser::read_next_token(void) noexcept
{
	_currentToken = _nextToken;
	_nextToken = _lexer.read_next_token(_scope);
	// 읽은 토큰이 주석이라면 주석이 아닐때까지 읽는다.
	while (_nextToken.Type == token_type::comment || _nextToken.Type == token_type::comment_block)
	{
		_nextToken = _lexer.read_next_token(_scope);
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

inline const bool mcf::parser::read_next_token_if(mcf::token_type tokenType, const std::string& scopeToPush) noexcept
{
	if (_nextToken.Type != tokenType)
	{
		return false;
	}
	_scope.emplace_back(scopeToPush);
	read_next_token();
	return true;
}

const bool mcf::parser::read_next_token_if_any(std::initializer_list<token_type> list) noexcept
{
	for (const token_type* iter = list.begin(); iter != list.end(); iter++)
	{
		if (_nextToken.Type == *iter)
		{
			read_next_token();
			return true;
		}
	}
	return false;
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
	case token_type::keyword_in: __COUNTER__;
	case token_type::keyword_out: __COUNTER__;
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

constexpr const size_t get_data_type_list_invalid = mcf::enum_index(mcf::token_type::invalid) + __COUNTER__;
constexpr const std::initializer_list<mcf::token_type> mcf::parser::get_data_type_list(void) const noexcept
{
	enum_at<token_type>(enum_index(token_type::eof) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::identifier) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::integer) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::string_utf8) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::assign) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::plus) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::minus) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::asterisk) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::slash) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::lt) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::gt) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::ampersand) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::lparen) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::rparen) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::lbrace) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::rbrace) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::lbracket) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::rbracket) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::colon) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::semicolon) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::comma) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::keyword_identifier_start) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::keyword_enum) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::keyword_unused) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::keyword_in) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::keyword_out) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::keyword_identifier_end) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::custom_keyword_start) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::keyword_identifier_end) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::keyword_identifier_end) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::keyword_variadic) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::macro_start) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::macro_iibrary_file_include) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::macro_project_file_include) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::macro_end) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::comment) + __COUNTER__ * 0);
	enum_at<token_type>(enum_index(token_type::comment_block) + __COUNTER__ * 0);
	return															    
	{ 																    
		enum_at<token_type>(enum_index(token_type::keyword_const) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_void) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_int8) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_int16) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_int32) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_int64) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_uint8) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_uint16) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_uint32) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_uint64) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::keyword_utf8) + __COUNTER__ * 0),
		enum_at<token_type>(enum_index(token_type::custom_enum_type) + __COUNTER__ * 0),
	};
}
constexpr const size_t get_data_type_list_count = +__COUNTER__ - get_data_type_list_invalid;
static_assert(get_data_type_list_count == mcf::enum_count<mcf::token_type>(), "token_type count is changed. this list need to be changed as well.");

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

	case token_type::rparen: __COUNTER__;
	case token_type::comma: __COUNTER__;
		break;

	case token_type::eof: __COUNTER__;
	case token_type::identifier: __COUNTER__;
	case token_type::integer: __COUNTER__;
	case token_type::string_utf8: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::ampersand: __COUNTER__;
	case token_type::lbrace: __COUNTER__;
	case token_type::rbrace: __COUNTER__;
	case token_type::rbracket: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
	case token_type::colon: __COUNTER__;
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
	case token_type::keyword_in: __COUNTER__;
	case token_type::keyword_out: __COUNTER__;
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
	debug_assert(inOutToken.Type == token_type::identifier || inOutToken.Type == token_type::custom_enum_type, 
		u8"identifier 또는 custom_enum_type 타입의 토큰만 커스텀 타입으로 변경 가능합니다.");
	inOutToken.Type = _lexer.register_custom_enum_type(inOutToken.Literal, _scope);
	parsing_fail_assert(inOutToken.Type == token_type::custom_enum_type, error::id::registering_duplicated_symbol_name, inOutToken, 
		u8"심볼이 중복되는 타입이 등록 되었습니다. 타입=%s", inOutToken.Literal.c_str());
	return true;
}
