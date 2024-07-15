#include "test/unittest/unittest.h"

const std::string UnitTest::convert_to_string(const mcf::token& token)
{
	std::string result;
	result += "mcf::token{Type : ";
	result += TOKEN_TYPES[mcf::enum_index(token.Type)];
	result += ", Literal : " + token.Literal + "}";
	return result;
}