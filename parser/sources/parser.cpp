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
	int pfa_bufferLength = snprintf(nullptr, 0, PARSING_FAIL_MESSAGE_FORMAT, TOKEN.Line, TOKEN.Index); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer == nullptr) { return false; } \
	snprintf(pfa_buffer, pfa_bufferLength + 1, PARSING_FAIL_MESSAGE_FORMAT, TOKEN.Line, TOKEN.Index); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
	pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer == nullptr) { return false; } \
	snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_message += "\n"; \
	mcf::parser::error pfa_error = { ERROR_ID, _name, pfa_message, TOKEN.Line, TOKEN.Index }; \
	_errors.push(pfa_error); \
	__debugbreak(); \
	return false;\
} ((void)0)
#define parsing_fail_message(ERROR_ID, TOKEN, FORMAT, ...) \
{ \
	std::string pfa_message; \
	int pfa_bufferLength = snprintf(nullptr, 0, PARSING_FAIL_MESSAGE_FORMAT, TOKEN.Line, TOKEN.Index); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer != nullptr) \
	{ \
		snprintf(pfa_buffer, pfa_bufferLength + 1, PARSING_FAIL_MESSAGE_FORMAT, TOKEN.Line, TOKEN.Index); \
		pfa_message += pfa_buffer; delete[] pfa_buffer; \
		pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
		pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
		if (pfa_buffer != nullptr) \
		{ \
			snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
			pfa_message += pfa_buffer; delete[] pfa_buffer; \
			pfa_message += "\n"; \
		} \
	} \
	mcf::parser::error pfa_error = { ERROR_ID, _name, pfa_message, TOKEN.Line, TOKEN.Index }; \
	_errors.push( pfa_error ); \
	__debugbreak(); \
} ((void)0)
#else
#define parsing_fail_assert(PREDICATE, ERROR_ID, TOKEN, FORMAT, ...) if ((PREDICATE) == false) \
{ \
	std::string pfa_message; \
	int pfa_bufferLength = snprintf(nullptr, 0, PARSING_FAIL_MESSAGE_FORMAT, TOKEN.Line, TOKEN.Index); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer == nullptr) { return false; } \
	snprintf(pfa_buffer, pfa_bufferLength + 1, PARSING_FAIL_MESSAGE_FORMAT, TOKEN.Line, TOKEN.Index); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
	pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer == nullptr) { return false; } \
	snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
	pfa_message += pfa_buffer; delete[] pfa_buffer; \
	pfa_message += "\n"; \
	mcf::parser::error pfa_error = { ERROR_ID, _name, pfa_message, TOKEN.Line, TOKEN.Index }; \
	_errors.push(pfa_error); \
	return false;\
} ((void)0)
#define parsing_fail_message(ERROR_ID, TOKEN, FORMAT, ...) \
{ \
	std::string pfa_message; \
	int pfa_bufferLength = snprintf(nullptr, 0, PARSING_FAIL_MESSAGE_FORMAT, TOKEN.Line, TOKEN.Index); \
	char* pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
	if (pfa_buffer != nullptr) \
	{ \
		snprintf(pfa_buffer, pfa_bufferLength + 1, PARSING_FAIL_MESSAGE_FORMAT, TOKEN.Line, TOKEN.Index); \
		pfa_message += pfa_buffer; delete[] pfa_buffer; \
		pfa_bufferLength = snprintf(nullptr, 0, FORMAT, __VA_ARGS__); \
		pfa_buffer = new(std::nothrow) char[pfa_bufferLength + 1]; \
		if (pfa_buffer != nullptr) \
		{ \
			snprintf(pfa_buffer, pfa_bufferLength + 1, FORMAT, __VA_ARGS__); \
			pfa_message += pfa_buffer; delete[] pfa_buffer; \
			pfa_message += "\n"; \
		} \
	} \
	mcf::parser::error pfa_error = { ERROR_ID, _name, pfa_message, TOKEN.Line, TOKEN.Index }; \
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
		constexpr const size_t TOKEN_TYPES_SIZE = sizeof(TOKEN_TYPES) / mcf::array_type_size(TOKEN_TYPES);
		static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPES_SIZE, "token_type count is changed. this VARIABLE need to be changed as well");

		inline static const std::string read_file(const std::string& path)
		{
			std::string input;
			{
				std::ifstream file(path.c_str());
				std::string line;
				while ( std::getline( file, line ) )
				{
					input += line;
				}
			}
			return input;
		}
	}
}

mcf::parser::parser(const std::string& input, const bool isFile) noexcept
	: _lexer(isFile ? internal::read_file(input) : input)
	, _name(input)
{
	_currentToken = _lexer.read_next_token();
	_nextToken = _lexer.read_next_token();
}

const size_t mcf::parser::get_error_count(void) noexcept
{
	return _errors.size();
}

const mcf::parser::error mcf::parser::get_last_error(void) noexcept
{
	if (_errors.empty())
	{
		return { parser::error::id::no_error, _name, std::string(), 0, 0};
	}

	const mcf::parser::error error = _errors.top();
	_errors.pop();
	return error;
}

void mcf::parser::parse_program(mcf::ast::program& outProgram) noexcept
{
	std::vector<const mcf::ast::statement*> unsafeProgram;
	while (_currentToken.Type != mcf::token_type::eof)
	{
		const mcf::ast::statement* statement = parse_statement();
		if (statement != nullptr)
		{
			unsafeProgram.emplace_back(statement);
		}

		// read next token
		read_next_token();
	}
	outProgram = mcf::ast::program(unsafeProgram);
}

inline const mcf::ast::statement* mcf::parser::parse_statement(void) noexcept
{
	std::unique_ptr<const ast::statement> statement;
	constexpr const size_t TOKEN_TYPE_COUNT_BEGIN = __COUNTER__;
	switch (_currentToken.Type)
	{
	// 데이터 타입 키워드인 경우
	case token_type::keyword_int32: __COUNTER__;
		statement = std::unique_ptr<const ast::statement>(parse_variable_declaration_statement());
		break;

	case token_type::identifier: __COUNTER__;
		statement = std::unique_ptr<const ast::statement>(parse_variable_assignment_statement());

	case token_type::eof: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
		break;

	case token_type::integer_32bit: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::plus: __COUNTER__;
	case token_type::minus: __COUNTER__;
	case token_type::asterisk: __COUNTER__;
	case token_type::slash: __COUNTER__;
	case token_type::lt: __COUNTER__;
	case token_type::gt: __COUNTER__;
	case token_type::lparen: __COUNTER__;
	case token_type::rparen: __COUNTER__;
	case token_type::lbrace: __COUNTER__;
	case token_type::rbrace: __COUNTER__;
	case token_type::lbracket: __COUNTER__;
	case token_type::rbracket: __COUNTER__;
	case token_type::comma: __COUNTER__;
	default:
		parsing_fail_message(error::id::not_registered_prefix_token, _currentToken, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu) literal=`%s`",
			internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type), _currentToken.Literal.c_str());
		break;
	}
	constexpr const size_t TOKEN_TYPE_COUNT = __COUNTER__ - TOKEN_TYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPE_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");
	return statement.release();
}

const mcf::ast::variable_declaration_statement* mcf::parser::parse_variable_declaration_statement(void) noexcept
{
	mcf::ast::data_type_expression dataType(_currentToken);

	if (read_next_token_if(token_type::identifier) == false)
	{
		return nullptr;
	}
	mcf::ast::identifier_expression	name(_currentToken);

	// optional
	std::unique_ptr<const mcf::ast::expression> rightExpression;
	if (_nextToken.Type == token_type::assign)
	{
		read_next_token();
		read_next_token();
		// TODO: expression 식 구현
		rightExpression = std::unique_ptr<const mcf::ast::expression>(parse_expression(mcf::parser::precedence::lowest));
		if (rightExpression == nullptr)
		{
			return nullptr;
		}
	}

	if (read_next_token_if(token_type::semicolon) == false)
	{
		return nullptr;
	}

	return new(std::nothrow) mcf::ast::variable_declaration_statement(dataType, name, rightExpression.release());
}

const mcf::ast::variable_assignment_statement* mcf::parser::parse_variable_assignment_statement(void) noexcept
{
	mcf::ast::identifier_expression	name(_currentToken);

	if (read_next_token_if(token_type::assign) == false)
	{
		return nullptr;
	}
	read_next_token();
	std::unique_ptr<const mcf::ast::expression> rightExpression = std::unique_ptr<const mcf::ast::expression>(parse_expression(mcf::parser::precedence::lowest));

	if (read_next_token_if(token_type::semicolon) == false)
	{
		return nullptr;
	}

	return new(std::nothrow) mcf::ast::variable_assignment_statement(name, rightExpression.release());
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
	
	case token_type::integer_32bit: __COUNTER__;
		expression = std::make_unique<ast::literal_expession>(_currentToken);
		break;

	case token_type::plus: __COUNTER__;
	case token_type::minus: __COUNTER__;
		expression = std::unique_ptr<const ast::expression>(parse_prefix_expression());
		break;

	case token_type::eof: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::asterisk: __COUNTER__;
	case token_type::slash: __COUNTER__;
	case token_type::lt: __COUNTER__;
	case token_type::gt: __COUNTER__;
	case token_type::lparen: __COUNTER__;
	case token_type::rparen: __COUNTER__;
	case token_type::lbrace: __COUNTER__;
	case token_type::rbrace: __COUNTER__;
	case token_type::lbracket: __COUNTER__;
	case token_type::rbracket: __COUNTER__;
	case token_type::keyword_int32: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
	case token_type::comma: __COUNTER__;
	default:
		parsing_fail_message(error::id::not_registered_prefix_token, _currentToken, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu)",
			internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type));
		break;
	}
	constexpr const size_t EXPRESSION_TOKEN_COUNT = __COUNTER__ - EXPRESSION_TOKEN_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::token_type::count) == EXPRESSION_TOKEN_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");

	while (expression.get() != nullptr && _nextToken.Type != token_type::semicolon && precedence < get_next_precedence())
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

		case token_type::eof: __COUNTER__;
		case token_type::identifier: __COUNTER__;
		case token_type::integer_32bit: __COUNTER__;
		case token_type::assign: __COUNTER__;
		case token_type::rparen: __COUNTER__;
		case token_type::lbrace: __COUNTER__;
		case token_type::rbrace: __COUNTER__;
		case token_type::rbracket: __COUNTER__;
		case token_type::semicolon: __COUNTER__;
		case token_type::comma: __COUNTER__;
		case token_type::keyword_int32: __COUNTER__;
		default:
			parsing_fail_message(error::id::not_registered_infix_token, _currentToken, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu)",
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)], enum_index(_nextToken.Type));
			return nullptr;
		}
		constexpr const size_t INFIX_COUNT = __COUNTER__ - INFIX_COUNT_BEGIN;
		static_assert(static_cast<size_t>(mcf::token_type::count) == INFIX_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");
	}

	return expression.release();
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
	const precedence precedence = get_current_token_precedence();
	read_next_token();
	std::unique_ptr<const ast::expression> rightExpression(parse_expression(precedence));
	if (rightExpression.get() == nullptr)
	{
		return nullptr;
	}
	return new(std::nothrow) ast::infix_expression(leftExpression.release(), infixOperatorToken, rightExpression.release());
}

const mcf::ast::infix_expression* mcf::parser::parse_call_expression(const mcf::ast::expression* left) noexcept
{
	unused(left);
	parsing_fail_message(error::id::not_registered_infix_token, token(), u8"#18 기본적인 평가기 개발 구현 필요");
	return nullptr;
}

const mcf::ast::infix_expression* mcf::parser::parse_index_expression(const mcf::ast::expression* left) noexcept
{
	unused(left);
	parsing_fail_message(error::id::not_registered_infix_token, token(), u8"#18 기본적인 평가기 개발 구현 필요");
	return nullptr;
}

inline void mcf::parser::read_next_token(void) noexcept
{
	_currentToken = _nextToken;
	_nextToken = _lexer.read_next_token();
}

inline const bool mcf::parser::read_next_token_if(mcf::token_type tokenType) noexcept
{
	parsing_fail_assert(_nextToken.Type == tokenType, error::id::unexpected_next_token, _nextToken, u8"다음 토큰은 %s여야만 합니다. 실제 값으로 %s를 받았습니다.",
		internal::TOKEN_TYPES[enum_index(tokenType)], internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
	read_next_token();
	return true;
}

inline const mcf::parser::precedence mcf::parser::get_expression_precedence(const mcf::token& token) noexcept
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
	case token_type::integer_32bit: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::rparen: __COUNTER__;
	case token_type::lbrace: __COUNTER__;
	case token_type::rbrace: __COUNTER__;
	case token_type::rbracket: __COUNTER__;
	case token_type::keyword_int32: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
	case token_type::comma: __COUNTER__;
	default:
		parsing_fail_message(parser::error::id::not_registered_infix_token, token, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu)",
			internal::TOKEN_TYPES[enum_index(token.Type)], enum_index(token.Type));
		break;
	}
	constexpr const size_t PRECEDENCE_COUNT = __COUNTER__ - PRECEDENCE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(token_type::count) == PRECEDENCE_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");

	return parser::precedence::lowest;
}

inline const mcf::parser::precedence mcf::parser::get_next_precedence(void) noexcept
{
	return get_expression_precedence(_nextToken);
}

inline const mcf::parser::precedence mcf::parser::get_current_token_precedence(void) noexcept
{
	return get_expression_precedence(_currentToken);
}
