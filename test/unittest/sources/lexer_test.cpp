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
					"=+-*/<>(){}[],;",
					{
						{mcf::token_type::assign, "="},
						{mcf::token_type::plus, "+"},
						{mcf::token_type::minus, "-"},
						{mcf::token_type::asterisk, "*"},
						{mcf::token_type::slash, "/"},
						{mcf::token_type::lt, "<"},
						{mcf::token_type::gt, ">"},
						{mcf::token_type::lparen, "("},
						{mcf::token_type::rparen, ")"},
						{mcf::token_type::lbrace, "{"},
						{mcf::token_type::rbrace, "}"},
						{mcf::token_type::lbracket, "["},
						{mcf::token_type::rbracket, "]"},
						{mcf::token_type::comma, ","},
						{mcf::token_type::semicolon, ";"},
					},
				},
				{
					"int32 foo = 5;enum PRINT_RESULT : int32{NO_ERROR}; const PRINT_RESULT Print(const utf8 format[], ...) const;",
					{
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::identifier, "foo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::integer_32bit, "5"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::keyword_enum, "enum"},
						{mcf::token_type::identifier, "PRINT_RESULT"},
						{mcf::token_type::colon, ":"},
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::lbrace, "{"},
						{mcf::token_type::identifier, "NO_ERROR"},
						{mcf::token_type::rbrace, "}"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::identifier, "PRINT_RESULT"},
						{mcf::token_type::identifier, "Print"},
						{mcf::token_type::lparen, "("},
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::keyword_utf8, "utf8"},
						{mcf::token_type::identifier, "format"},
						{mcf::token_type::lbracket, "["},
						{mcf::token_type::rbracket, "]"},
						{mcf::token_type::comma, ","},
						{mcf::token_type::keyword_variadic, "..."},
						{mcf::token_type::rparen, ")"},
						{mcf::token_type::keyword_const, "const"},
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
					"int32 foo = -1;     ",
					{
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::identifier, "foo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::minus, "-"},
						{mcf::token_type::integer_32bit, "1"},
						{mcf::token_type::semicolon, ";"},
					},
				},
				{
					"#include <builtins>// testing comment; int32 boo = -1; //",
					{
						{mcf::token_type::macro_iibrary_file_include, "#include <builtins>"},
						{mcf::token_type::comment, "// testing comment; int32 boo = -1; //"},
					},
				},
				{
					"#include <builtins>/*// testing comment;*/ int32 boo = -1; // hello world",
					{
						{mcf::token_type::macro_iibrary_file_include, "#include <builtins>"},
						{mcf::token_type::comment_block, "/*// testing comment;*/"},
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::identifier, "boo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::minus, "-"},
						{mcf::token_type::integer_32bit, "1"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::comment, "// hello world"},
					},
				},
			};
			constexpr const size_t testCaseCount = array_size(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				const size_t vectorSize = testCases[i].ExpectedResultVector.size();
				mcf::lexer lexer(testCases[i].Input, false);
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

		_names.emplace_back("./test/unittest/texts/test_file_read.txt");
		_tests.emplace_back([&]() {
			struct expected_result final
			{
				const mcf::token_type   Type;
				const char*				Literal;
			};

			const std::vector<expected_result>  expectedResultVector =
			{
					{mcf::token_type::keyword_int32, "int32"},
					{mcf::token_type::identifier, "foo"},
					{mcf::token_type::assign, "="},
					{mcf::token_type::integer_32bit, "10"},
					{mcf::token_type::semicolon, ";"},
					{mcf::token_type::comment, "//\\"},
					{mcf::token_type::comment, u8"// TODO: 주석의 마지막이 \"\\\\n\" 라면 \\n을 escape 한다. 해당 구현 완료되면 이 주석의 \"//\"를 제거 하세요."},
					{mcf::token_type::keyword_int32, "int32"},
					{mcf::token_type::identifier, "boo"},
					{mcf::token_type::assign, "="},
					{mcf::token_type::integer_32bit, "5"},
					{mcf::token_type::semicolon, ";"},
			};
			const size_t expectedResultVectorSize = expectedResultVector.size();
			mcf::lexer lexer(_names.back().c_str(), true);
			for (size_t i = 0; i < expectedResultVectorSize; i++)
			{
				const mcf::token token = lexer.read_next_token();
				const mcf::token_type expectedTokenType = expectedResultVector[i].Type;

				fatal_assert(token.Type == expectedTokenType, u8"tests[line: %zu, index: %zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
					token.Line, token.Index, TOKEN_TYPES[enum_index(expectedTokenType)], TOKEN_TYPES[enum_index(token.Type)]);

				fatal_assert(token.Literal == expectedResultVector[i].Literal, u8"tests[line: %zu, index: %zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
					token.Line, token.Index, expectedResultVector[i].Literal, token.Literal.c_str());
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