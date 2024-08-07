#include <iostream>
#include <vector>
#include <string>

#include "../unittest.h"

namespace UnitTest
{
	Lexer::Lexer(void) noexcept
	{
		_names.emplace_back("test_generated_tokens_by_lexer");
		_tests.emplace_back([&]() {
			const struct test_case
			{
				const std::string                   Input;
				const std::vector<mcf::token>  ExpectedResults;
			} testCases[] =
			{
				{ // 0. 연산자, 구분자, 및 블록 지정자
					"=+-*/<>(){}[]:;,",
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
						{mcf::token_type::colon, ":"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::comma, ","},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 1. 리터럴
					"12345 \"hello, world!\"",
					{
						{mcf::token_type::integer, "12345"},
						{mcf::token_type::string_utf8, "\"hello, world!\""},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 2. 식별자 키워드
					"const void int8 int16 int32 int64 uint8 uint16 uint32 uint64 utf8 enum unused",
					{
						token_const,
						token_void,
						token_int8,
						token_int16,
						token_int32,
						token_int64,
						token_uint8,
						token_uint16,
						token_uint32,
						token_uint64,
						token_utf8,
						{mcf::token_type::keyword_enum, "enum"},
						{mcf::token_type::keyword_unused, "unused"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 3. '.' 으로 시작하는 토큰
					"...",
					{
						{mcf::token_type::keyword_variadic, "..."},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 4. 매크로
					"#include <hello, world!>\n#include \"custom_file.hmcf\"",
					{
						{mcf::token_type::macro_iibrary_file_include, "#include <hello, world!>"},
						{mcf::token_type::macro_project_file_include, "#include \"custom_file.hmcf\""},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 5. 주석
					u8"// 한줄 주석입니다.\n/* 여러 줄을 주석\n 처리\n 할수 있습니다. */",
					{
						{mcf::token_type::comment, u8"// 한줄 주석입니다."},
						{mcf::token_type::comment_block, u8"/* 여러 줄을 주석\n 처리\n 할수 있습니다. */"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 6. 변수 관련 토큰
					"int32 foo = 1;uint8 boo = -7;",
					{
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::identifier, "foo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::integer, "1"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::keyword_uint8, "uint8"},
						{mcf::token_type::identifier, "boo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::minus, "-"},
						{mcf::token_type::integer, "7"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 7. enum 관련 토큰
					"enum PRINT_RESULT : uint8{NO_ERROR};enum PRINT_RESULT2 : int8{INVALID,ERROR1=1,ERROR2};enum PRINT_RESULT3 : int8{INVALID=0,ERROR1,ERROR2,COUNT,};",
					{
						{mcf::token_type::keyword_enum, "enum"},
						{mcf::token_type::identifier, "PRINT_RESULT"},
						{mcf::token_type::colon, ":"},
						{mcf::token_type::keyword_uint8, "uint8"},
						{mcf::token_type::lbrace, "{"},
						{mcf::token_type::identifier, "NO_ERROR"},
						{mcf::token_type::rbrace, "}"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::keyword_enum, "enum"},
						{mcf::token_type::identifier, "PRINT_RESULT2"},
						{mcf::token_type::colon, ":"},
						{mcf::token_type::keyword_int8, "int8"},
						{mcf::token_type::lbrace, "{"},
						{mcf::token_type::identifier, "INVALID"},
						{mcf::token_type::comma, ","},
						{mcf::token_type::identifier, "ERROR1"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::integer, "1"},
						{mcf::token_type::comma, ","},
						{mcf::token_type::identifier, "ERROR2"},
						{mcf::token_type::rbrace, "}"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::keyword_enum, "enum"},
						{mcf::token_type::identifier, "PRINT_RESULT3"},
						{mcf::token_type::colon, ":"},
						{mcf::token_type::keyword_int8, "int8"},
						{mcf::token_type::lbrace, "{"},
						{mcf::token_type::identifier, "INVALID"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::integer, "0"},
						{mcf::token_type::comma, ","},
						{mcf::token_type::identifier, "ERROR1"},
						{mcf::token_type::comma, ","},
						{mcf::token_type::identifier, "ERROR2"},
						{mcf::token_type::comma, ","},
						{mcf::token_type::identifier, "COUNT"},
						{mcf::token_type::comma, ","},
						{mcf::token_type::rbrace, "}"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 8. 함수 전방 선언 관련 토큰
					"const PRINT_RESULT Print(const utf8 format[], ...) const;",
					{
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
				{ // 9. #include 메크로
					"#include <builtins> // include vector, string, print",
					{
						{mcf::token_type::macro_iibrary_file_include, "#include <builtins>"},
						{mcf::token_type::comment, "// include vector, string, print"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 10. 함수 선언 토큰
					"void main(void) { const utf8 str[] = \"Hello, World!\"; /* default string literal is static array of utf8 in mcf */ Print(\"%s\\n\", str); }",
					{
						{mcf::token_type::keyword_void, "void"},
						{mcf::token_type::identifier, "main"},
						{mcf::token_type::lparen, "("},
						{mcf::token_type::keyword_void, "void"},
						{mcf::token_type::rparen, ")"},
						{mcf::token_type::lbrace, "{"},
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::keyword_utf8, "utf8"},
						{mcf::token_type::identifier, "str"},
						{mcf::token_type::lbracket, "["},
						{mcf::token_type::rbracket, "]"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::string_utf8, "\"Hello, World!\""},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::comment_block, "/* default string literal is static array of utf8 in mcf */"},
						{mcf::token_type::identifier, "Print"},
						{mcf::token_type::lparen, "("},
						{mcf::token_type::string_utf8, "\"%s\\n\""},
						{mcf::token_type::comma, ","},
						{mcf::token_type::identifier, "str"},
						{mcf::token_type::rparen, ")"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::rbrace, "}"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 11. 문자열
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
				{ // 12. slash eof
					"/",
					{
						{mcf::token_type::slash, "/"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 13. 주석 eof
					"//",
					{
						{mcf::token_type::comment, "//"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 14. 주석 블록
					"const /* utf8  */ int32 comment_block_test = 5;",
					{
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::comment_block, "/* utf8  */"},
						{mcf::token_type::keyword_int32, "int32"},
						{mcf::token_type::identifier, "comment_block_test"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::integer, "5"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::eof, "\0"},
					},
				},
				{ // 15. bool 변수 및 비교/논리 연산자
					"const bool foo = false; const bool boo = true; const bool jar = foo == boo; const bool bar = jar != boo; const bool tar = !jar;",
					{
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::keyword_bool, "bool"},
						{mcf::token_type::identifier, "foo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::keyword_false, "false"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::keyword_bool, "bool"},
						{mcf::token_type::identifier, "boo"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::keyword_true, "true"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::keyword_bool, "bool"},
						{mcf::token_type::identifier, "jar"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::identifier, "foo"},
						{mcf::token_type::equal, "=="},
						{mcf::token_type::identifier, "boo"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::keyword_bool, "bool"},
						{mcf::token_type::identifier, "bar"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::identifier, "jar"},
						{mcf::token_type::not_equal, "!="},
						{mcf::token_type::identifier, "boo"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::keyword_const, "const"},
						{mcf::token_type::keyword_bool, "bool"},
						{mcf::token_type::identifier, "tar"},
						{mcf::token_type::assign, "="},
						{mcf::token_type::bang, "!"},
						{mcf::token_type::identifier, "jar"},
						{mcf::token_type::semicolon, ";"},
						{mcf::token_type::eof, "\0"},
					},
				},
			};
			constexpr const size_t testCaseCount = array_size(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				const size_t expectedResultSize = testCases[i].ExpectedResults.size();
				mcf::evaluator evaluator;
				mcf::lexer lexer(&evaluator, testCases[i].Input, false);

				std::vector<mcf::token>  actualTokens;
				mcf::token token = lexer.read_next_token();
				actualTokens.emplace_back(token);
				while (token.Type != mcf::token_type::eof || token.Type == mcf::token_type::invalid)
				{
					token = lexer.read_next_token();
					actualTokens.emplace_back(token);
				}
				const size_t actualTokenCount = actualTokens.size();
				fatal_assert(expectedResultSize == actualTokenCount, u8"기대값의 갯수와 실제 생성된 토큰의 갯수가 같아야 합니다. 예상값=%zu, 실제값=%zu", expectedResultSize, actualTokenCount);

				for (size_t j = 0; j < expectedResultSize; j++)
				{
					fatal_assert(actualTokens[j].Type == testCases[i].ExpectedResults[j].Type, u8"tests[%zu-%zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
						i, j, TOKEN_TYPES[enum_index(testCases[i].ExpectedResults[j].Type)], TOKEN_TYPES[enum_index(actualTokens[j].Type)]);

					fatal_assert(actualTokens[j].Literal == testCases[i].ExpectedResults[j].Literal, u8"tests[%zu-%zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
						i, j, testCases[i].ExpectedResults[j].Literal.c_str(), actualTokens[j].Literal.c_str());
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
					{mcf::token_type::integer, "10"},
					{mcf::token_type::semicolon, ";"},
					{mcf::token_type::comment, "//\\"},
					{mcf::token_type::comment, u8"// TODO: 주석의 마지막이 \"\\\\n\" 라면 \\n을 escape 한다. 해당 구현 완료되면 이 주석의 \"//\"를 제거 하세요."},
					{mcf::token_type::keyword_int32, "int32"},
					{mcf::token_type::identifier, "boo"},
					{mcf::token_type::assign, "="},
					{mcf::token_type::integer, "5"},
					{mcf::token_type::semicolon, ";"},
					{mcf::token_type::comment, u8"// unused parameters must be set as unused"},
					{mcf::token_type::comment, u8"// Print is builtin function that prints output to console"},
					{mcf::token_type::comment, u8"// TODO: command line argument 받는법 필요"},
					{mcf::token_type::comment, u8"// TODO: return type for main?"},
					{mcf::token_type::keyword_enum, u8"enum"},
					{mcf::token_type::identifier, "PRINT_RESULT"},
					{mcf::token_type::lbrace, "{"},
					{mcf::token_type::identifier, "NO_ERROR"},
					{mcf::token_type::rbrace, "}"},
					{mcf::token_type::semicolon, ";"},
					{mcf::token_type::keyword_const, "const"},
					{mcf::token_type::identifier, "PRINT_RESULT"},
					{mcf::token_type::identifier, "Print"},
					{mcf::token_type::lparen, "("},
					{mcf::token_type::keyword_in, "in"},
					{mcf::token_type::keyword_const, "const"},
					{mcf::token_type::keyword_utf8, "utf8"},
					{mcf::token_type::identifier, "format"},
					{mcf::token_type::lbracket, "["},
					{mcf::token_type::rbracket, "]"},
					{mcf::token_type::comma, ","},
					{mcf::token_type::keyword_variadic, "..."},
					{mcf::token_type::rparen, ")"},
					{mcf::token_type::semicolon, ";"},
					{mcf::token_type::keyword_void, "void"},
					{mcf::token_type::identifier, "main"},
					{mcf::token_type::lparen, "("},
					{mcf::token_type::keyword_unused, "unused"},
					{mcf::token_type::keyword_const, "const"},
					{mcf::token_type::keyword_int32, "int32"},
					{mcf::token_type::identifier, "argc"},
					{mcf::token_type::comma, ","},
					{mcf::token_type::keyword_unused, "unused"},
					{mcf::token_type::keyword_const, "const"},
					{mcf::token_type::keyword_utf8, "utf8"},
					{mcf::token_type::identifier, "argv"},
					{mcf::token_type::lbracket, "["},
					{mcf::token_type::rbracket, "]"},
					{mcf::token_type::lbracket, "["},
					{mcf::token_type::rbracket, "]"},
					{mcf::token_type::rparen, ")"},
					{mcf::token_type::lbrace, "{"},
					{mcf::token_type::keyword_enum, u8"enum"},
					{mcf::token_type::identifier, "PRINT_RESULT"},
					{mcf::token_type::lbrace, "{"},
					{mcf::token_type::identifier, "NO_ERROR"},
					{mcf::token_type::rbrace, "}"},
					{mcf::token_type::semicolon, ";"},
					{mcf::token_type::keyword_const, "const"},
					{mcf::token_type::keyword_utf8, "utf8"},
					{mcf::token_type::identifier, "str"},
					{mcf::token_type::lbracket, "["},
					{mcf::token_type::rbracket, "]"},
					{mcf::token_type::assign, "="},
					{mcf::token_type::string_utf8, "\"Hello, World!\""},
					{mcf::token_type::semicolon, ";"},
					{mcf::token_type::comment, "// default string literal is static array of utf8 in mcf"},
					{mcf::token_type::identifier, "Print"},
					{mcf::token_type::lparen, "("},
					{mcf::token_type::string_utf8, "\"%s\\n\""},
					{mcf::token_type::comma, ","},
					{mcf::token_type::identifier, "str"},
					{mcf::token_type::rparen, ")"},
					{mcf::token_type::semicolon, ";"},
					{mcf::token_type::rbrace, "}"},
					{mcf::token_type::eof, "\0"},
			};
			const size_t expectedResultSize = expectedResults.size();

			mcf::evaluator evaluator;
			mcf::lexer lexer(&evaluator, _names.back().c_str(), true);

			std::vector<mcf::token>  actualTokens;
			mcf::token token = lexer.read_next_token();
			actualTokens.emplace_back(token);
			while (token.Type != mcf::token_type::eof || token.Type == mcf::token_type::invalid)
			{
				token = lexer.read_next_token();
				actualTokens.emplace_back(token);
			}
			const size_t actualTokenCount = actualTokens.size();
			fatal_assert(expectedResultSize == actualTokenCount, u8"기대값의 갯수와 실제 생성된 토큰의 갯수가 같아야 합니다. 예상값=%zu, 실제값=%zu", expectedResultSize, actualTokenCount);

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