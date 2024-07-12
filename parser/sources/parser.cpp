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
		static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPES_SIZE, u8"토큰의 갯수가 일치 하지 않습니다. 수정이 필요합니다!");
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

void mcf::parser::parse_program(std::vector<const mcf::ast::statement*>& outProgram) noexcept
{
	while (_currentToken.Type != mcf::token_type::eof)
	{
		const mcf::ast::statement* lStatement = nullptr;
		// parse statement
		{
			switch (_currentToken.Type)
			{
			case mcf::token_type::keyword_int32:
				static_assert(enum_count(mcf::token_type) - enum_index(mcf::token_type::keyword_int32) == 1, u8"키워드 타입의 갯수가 변경 되었습니다. 데이터 타입이라면 수정해주세요!");
				lStatement = parse_variable_declaration_statement();
				break;
			default:
				break;
			}
		}
		if (lStatement != nullptr)
		{
			outProgram.emplace_back(lStatement);
		}

		// read next token
		read_next_toekn();
	}
}

const mcf::ast::variable_declaration_statement* mcf::parser::parse_variable_declaration_statement() noexcept
{
	mcf::ast::data_type_expression lDataType(_currentToken);

	if (read_next_token_if(token_type::identifier) == false)
	{
		return nullptr;
	}
	mcf::ast::identifier_expression	lName(_currentToken);

	// optional
	mcf::ast::expression* lRightExpression = nullptr;
	if (_nextToken.Type == token_type::assign)
	{
		read_next_toekn();
		// TODO: expression 식 구현
	}

	while (_currentToken.Type != token_type::semicolon)
	{
		read_next_toekn();
	}

	return new(std::nothrow) mcf::ast::variable_declaration_statement(lDataType, lName, lRightExpression);
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
