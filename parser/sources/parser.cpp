// parser.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "parser.h"

// TODO: This is an example of a library function
void fnparser()
{
}

mcf::parser::parser(const std::string& input) noexcept
	: _lexer(input)
{
	_currentToken = _lexer.read_next_token();
	_nextToken = _lexer.read_next_token();
}

mcf::ast::program* mcf::parser::parse_program( void ) noexcept
{
	ast::program* program = new ast::program();
	program;
	return nullptr;
}

inline void mcf::parser::read_next_token(void) noexcept
{
	_currentToken = _nextToken;
	_nextToken = _lexer.read_next_token();
}
