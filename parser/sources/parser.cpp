// parser.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "ast.h"
#include "parser.h"

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

			// 구분자
			"semicolon",

			// 예약어
			"keyword",
		};
		constexpr const size_t TOKEN_TYPES_SIZE = array_size(TOKEN_TYPES);
		static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPES_SIZE, "token_type count is changed. this VARIABLE need to be changed as well");
	}
}


mcf::parser::parser(const std::string& input) noexcept
	: _lexer(input)
{
	_currentToken = _lexer.read_next_token();
	_nextToken = _lexer.read_next_token();
}

const size_t mcf::parser::get_error_count(void) noexcept
{
	return internal::PARSER_ERRORS.size();
}

const mcf::parser::error mcf::parser::get_last_error(void) noexcept
{
	if (internal::PARSER_ERRORS.empty())
	{
		return { parser::error::id::no_error, std::string() };
	}

	const mcf::parser::error error = internal::PARSER_ERRORS.top();
	internal::PARSER_ERRORS.pop();
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
		read_next_toekn();
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

	case token_type::eof: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
		break;

	case token_type::invalid: __COUNTER__;
	case token_type::identifier: __COUNTER__;
	case token_type::integer_32bit: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::plus: __COUNTER__;
	case token_type::minus: __COUNTER__;
	case token_type::asterisk: __COUNTER__;
	case token_type::slash: __COUNTER__;
	default:
		parsing_fail_message(error::id::not_registered_prefix_token, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu)", internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type));
		break;
	}
	constexpr const size_t TOKEN_TYPE_COUNT = __COUNTER__ - TOKEN_TYPE_COUNT_BEGIN - 1;
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
		read_next_toekn();
		read_next_toekn();
		// TODO: expression 식 구현
		rightExpression = std::unique_ptr<const mcf::ast::expression>(parse_expression(mcf::parser::expression_precedence::lowest));
		if (rightExpression == nullptr)
		{
			return nullptr;
		}
	}

	while (_currentToken.Type != token_type::semicolon)
	{
		if (_currentToken.Type == token_type::eof || _currentToken.Type == token_type::invalid)
		{
			parsing_fail_message(error::id::not_registered_prefix_token, 
				u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. _currentToken.Type=%s(%zu) _nextToken.Type=%s(%zu)", 
				internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type),
				internal::TOKEN_TYPES[enum_index(_nextToken.Type)], enum_index(_nextToken.Type));
		}
		read_next_toekn();
	}

	return new(std::nothrow) mcf::ast::variable_declaration_statement(dataType, name, rightExpression.release());
}

const mcf::ast::expression* mcf::parser::parse_expression(const mcf::parser::expression_precedence precedence) noexcept
{
	precedence;
	std::unique_ptr<ast::expression> expression;
	constexpr const size_t PREFIX_COUNT_BEGIN = __COUNTER__;
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
		//expression = parse_prefix_expression();
		break;

	case token_type::invalid: __COUNTER__;
	case token_type::eof: __COUNTER__;
	case token_type::assign: __COUNTER__;
	case token_type::asterisk: __COUNTER__;
	case token_type::slash: __COUNTER__;
	case token_type::keyword_int32: __COUNTER__;
	case token_type::semicolon: __COUNTER__;
	default:
		parsing_fail_message(error::id::not_registered_prefix_token, u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. token_type=%s(%zu)", internal::TOKEN_TYPES[enum_index(_currentToken.Type)], enum_index(_currentToken.Type));
		break;
	}
	constexpr const size_t PREFIX_COUNT = __COUNTER__ - PREFIX_COUNT_BEGIN - 1;
	static_assert(static_cast<size_t>(mcf::token_type::count) == PREFIX_COUNT, "token_type count is changed. this SWITCH need to be changed as well.");

	return expression.release();
}

inline void mcf::parser::read_next_toekn(void) noexcept
{
	_currentToken = _nextToken;
	_nextToken = _lexer.read_next_token();
}

inline const bool mcf::parser::read_next_token_if(mcf::token_type tokenType) noexcept
{
	parsing_fail_assert(_nextToken.Type == tokenType, error::id::unexpected_next_token, u8"다음 토큰은 %s여야만 합니다. 실제 값으로 %s를 받았습니다.", 
		internal::TOKEN_TYPES[enum_index(tokenType)], internal::TOKEN_TYPES[enum_index(_nextToken.Type)]);
	read_next_toekn();
	return true;
}
