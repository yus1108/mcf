﻿#include <iostream>
#include <filesystem>
#include "test/unittest/unittest.h"

UnitTest::Evaluator::Evaluator(void) noexcept
{
	constexpr const char* const symbol_share_test = "./test/unittest/texts/symbol_share_test.mcf";
	_names.emplace_back(symbol_share_test);
	_tests.emplace_back
	(
		[symbol_share_test]()
		{
			mcf::ast::program actualProgram;
			mcf::evaluator evaluator;
			mcf::parser parser(&evaluator, symbol_share_test, true);
			mcf::parser::error parserInitError = parser.get_last_error();
			fatal_assert(parserInitError.ID == mcf::parser::error::id::no_error, "ID=`%s`, File=`%s`(%zu, %zu)\n%s",
				PARSER_ERROR_ID[enum_index(parserInitError.ID)], parserInitError.Name.c_str(), parserInitError.Line, parserInitError.Index, parserInitError.Message.c_str());
			parser.parse_program(actualProgram);
			return Parser::check_parser_errors(parser);
		}
	);
}

const bool UnitTest::Evaluator::Test(void) const noexcept
{
	for (size_t i = 0; i < _tests.size(); i++)
	{
		if (_tests[i]() == false)
		{
			std::cout << "Test[#" << i << "] `" << _names[i] << "` Failed" << std::endl;
			return false;
		}
		std::cout << "Parser Test[#" << i << "] `" << _names[i] << "` Passed" << std::endl;
	}
	return true;
}