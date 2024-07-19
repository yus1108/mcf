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
				const std::vector<expected_result>  ExpectedResults;
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
						{mcf::token_type::eof, "\0"},
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
						{mcf::token_type::eof, "\0"},
					},
				},
				{
					"int32 foo = 5 + 5 - 8 * 4 / 2;/      ",
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
						{mcf::token_type::slash, "/"},
						{mcf::token_type::eof, "\0"},
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
						{mcf::token_type::eof, "\0"},
					},
				},
				{
					"#include <builtins>// testing comment; int32 boo = -1; //",
					{
						{mcf::token_type::macro_iibrary_file_include, "#include <builtins>"},
						{mcf::token_type::comment, "// testing comment; int32 boo = -1; //"},
						{mcf::token_type::eof, "\0"},
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
						{mcf::token_type::eof, "\0"},
					},
				},
				{
					"const utf8 str[] = \"Hello, World!\"; // default string literal is static array of utf8 in mcf",
					{
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::keyword_utf8, "utf8"},
						{mcf::token_type::identifier, "str"},
						{mcf::token_type::lbracket, "["},
						{mcf::token_type::rbracket, "]"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::string_utf8, "\"Hello, World!\""},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::comment, "// default string literal is static array of utf8 in mcf"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{
					"/",
					{
						{mcf::token_type::slash, "/"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{
					"//",
					{
						{mcf::token_type::comment, "//"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{
					"const /* utf8  */ int32 comment_block_test = 5;",
					{
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::comment_block, "/* utf8  */"},
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::identifier, "comment_block_test"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::integer_32bit, "5"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::eof, "\0"},
					},
				},
			};
			constexpr const size_t testCaseCount = array_size(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				const size_t expectedResultSize = testCases[i].ExpectedResults.size();
				mcf::lexer lexer(testCases[i].Input, false);

				std::vector<mcf::token>  actualTokens;
				mcf::token token = lexer.read_next_token();
				actualTokens.emplace_back(token);
				while (token.Type != mcf::token_type::eof || token.Type == mcf::token_type::invalid)
				{
					token = lexer.read_next_token();
					actualTokens.emplace_back(token);
				}
				const size_t actualTokenCount = actualTokens.size();
				fatal_assert(expectedResultSize == actualTokenCount, "기대값의 갯수와 실제 생성된 토큰의 갯수가 같아야 합니다. 예상값=%zu, 실제값=%zu", expectedResultSize, actualTokenCount);

				for (size_t j = 0; j < expectedResultSize; j++)
				{
					fatal_assert(actualTokens[j].Type == testCases[i].ExpectedResults[j].Type, u8"tests[%zu-%zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
						i, j, TOKEN_TYPES[enum_index(testCases[i].ExpectedResults[j].Type)], TOKEN_TYPES[enum_index(actualTokens[j].Type)]);

					fatal_assert(actualTokens[j].Literal == testCases[i].ExpectedResults[j].Literal, u8"tests[%zu-%zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
						i, j, testCases[i].ExpectedResults[j].Literal, actualTokens[j].Literal.c_str());
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

			const std::vector<expected_result>  expectedResults =
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
					{mcf::token_type::comment, u8"// unused parameters must be set as unused"},
					{mcf::token_type::comment, u8"// Print is builtin function that prints output to console"},
					{mcf::token_type::comment, u8"// TODO: command line argument 받는법 필요"},
					{mcf::token_type::comment, u8"// TODO: return type for main?"},
					{mcf::token_type::keyword_enum, u8"enum"},
					{mcf::token_type::identifier, "PRINT_RESULT"},
					{mcf::token_type::colon, ":"},
					{mcf::token_type::keyword_uint8, "uint8"},
					{mcf::token_type::lbrace, "{"},
					{mcf::token_type::identifier, "NO_ERROR"},
					{mcf::token_type::rbrace, "}"},
					{mcf::token_type::semicolon, ";"},
					{mcf::token_type::eof, "\0"},
			};
			const size_t expectedResultSize = expectedResults.size();

			mcf::lexer lexer(_names.back().c_str(), true);

			std::vector<mcf::token>  actualTokens;
			mcf::token token = lexer.read_next_token();
			actualTokens.emplace_back(token);
			while (token.Type != mcf::token_type::eof || token.Type == mcf::token_type::invalid)
			{
				token = lexer.read_next_token();
				actualTokens.emplace_back(token);
			}
			const size_t actualTokenCount = actualTokens.size();
			fatal_assert(expectedResultSize == actualTokenCount, "기대값의 갯수와 실제 생성된 토큰의 갯수가 같아야 합니다. 예상값=%zu, 실제값=%zu", expectedResultSize, actualTokenCount);

			for (size_t i = 0; i < actualTokenCount; i++)
			{
				fatal_assert(actualTokens[i].Type == expectedResults[i].Type, u8"tests[line: %zu, index: %zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
					actualTokens[i].Line, actualTokens[i].Index, TOKEN_TYPES[enum_index(expectedResults[i].Type)], TOKEN_TYPES[enum_index(actualTokens[i].Type)]);

				fatal_assert(actualTokens[i].Literal == expectedResults[i].Literal, u8"tests[line: %zu, index: %zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
					actualTokens[i].Line, actualTokens[i].Index, expectedResults[i].Literal, actualTokens[i].Literal.c_str());
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