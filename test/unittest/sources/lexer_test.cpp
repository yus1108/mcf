#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include "../unittest.h"

namespace UnitTest
{
	Lexer::Lexer(void) noexcept
	{
		_names.emplace_back("test_generated_tokens_by_lexer");
		_tests.emplace_back([&]() {
			struct expected_result final
			{
				const mcf::token_type   Type;
				const char* Literal;
			};

			const struct test_case
			{
				const std::string                   Input;
				const std::vector<expected_result>  ExpectedResultVector;
			} testCases[] =
			{
				{
					// TODO: constexpr std::string_view lInput = "=+-*/(){},;";
					"=+-*/;",
					{
						{mcf::token_type::assign, "="},
						{mcf::token_type::plus, "+"},
						{mcf::token_type::minus, "-"},
						{mcf::token_type::asterisk, "*"},
						{mcf::token_type::slash, "/"},
						// TODO: {mcf::token_type::lparen, "("},
						// TODO: {mcf::token_type::rparen, ")"},
						// TODO: {mcf::token_type::lbrace, "{"},
						// TODO: {mcf::token_type::rbrace, "}"},
						// TODO: {mcf::token_type::comma, ","},
						{mcf::token_type::semicolon, ";"},
					},
				},
				{
					"int32 foo = 5;",
					{
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::identifier, "foo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::integer_32bit, "5"},
						{mcf::token_type::semicolon, ";"},
					},
				},
				{
					"int32 foo = 5 + 5 - 8 * 4 / 2;",
					{
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::identifier, "foo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::integer_32bit, "5"},
						{mcf::token_type::plus, "+"},
						{mcf::token_type::integer_32bit, "5"},
						{mcf::token_type::minus, "-"},
						{mcf::token_type::integer_32bit, "8"},
						{mcf::token_type::asterisk, "*"},
						{mcf::token_type::integer_32bit, "4"},
						{mcf::token_type::slash, "/"},
						{mcf::token_type::integer_32bit, "2"},
						{mcf::token_type::semicolon, ";"},
					},
				},
				{
					"int32 foo = -1;",
					{
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::identifier, "foo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::minus, "-"},
						{mcf::token_type::integer_32bit, "1"},
						{mcf::token_type::semicolon, ";"},
					},
				},
			};
			constexpr const size_t testCaseCount = array_size(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				const size_t vectorSize = testCases[i].ExpectedResultVector.size();
				mcf::lexer lexer(testCases[i].Input);
				for (size_t j = 0; j < vectorSize; j++)
				{
					const mcf::token token = lexer.read_next_token();
					const mcf::token_type expectedTokenType = testCases[i].ExpectedResultVector[j].Type;

					fatal_assert(token.Type == expectedTokenType, u8"tests[%zu-%zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
						i, j, TOKEN_TYPES[enum_index(expectedTokenType)], TOKEN_TYPES[enum_index(token.Type)]);

					fatal_assert(token.Literal == testCases[i].ExpectedResultVector[j].Literal, u8"tests[%zu-%zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
						i, j, testCases[i].ExpectedResultVector[j].Literal, token.Literal.c_str());
				}
			}

			return true;
		});
	}

	const bool Lexer::Test(void) const noexcept
	{
		for (size_t i = 0; i < _tests.size(); i++)
		{
			if (_tests[i]() == false)
			{
				std::cout << "Test[#" << i << "] `" << _names[i] << "()` Failed" << std::endl;
				return false;
			}
			std::cout << "Lexer Test[#" << i << "] `" << _names[i] << "()` Passed" << std::endl;
		}
		return true;
	}
}