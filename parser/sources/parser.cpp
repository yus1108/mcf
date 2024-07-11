// parser.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "ast.h"
#include "parser.h"

mcf::parser::parser(const std::string& input) noexcept
	: _lexer(input)
{
	_currentToken = _lexer.read_next_token();
	_nextToken = _lexer.read_next_token();
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
				lStatement = parse_variable_declaration_statement();
				break;
			default:
				break;
			}
			static_assert(ENUM_COUNT(mcf::token_type) - ENUM_INDEX(mcf::token_type::keyword_int32) == 1, u8"키워드 타입의 갯수가 변경 되었습니다. 데이터 타입이라면 수정해주세요!");
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

	if (_nextToken.Type != token_type::identifier)
	{
		return nullptr;
	}
	read_next_toekn();
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