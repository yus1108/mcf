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
				const std::vector<mcf::Token::Data>  ExpectedResults;
			} testCases[] =
			{
				{ // 0. 연산자, 구분자, 및 블록 지정자
					"=+-*/<>(){}[]:;,",
					{
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::PLUS, "+"},
						{mcf::Token::Type::MINUS, "-"},
						{mcf::Token::Type::ASTERISK, "*"},
						{mcf::Token::Type::SLASH, "/"},
						{mcf::Token::Type::LT, "<"},
						{mcf::Token::Type::GT, ">"},
						{mcf::Token::Type::LPAREN, "("},
						{mcf::Token::Type::RPAREN, ")"},
						{mcf::Token::Type::LBRACE, "{"},
						{mcf::Token::Type::RBRACE, "}"},
						{mcf::Token::Type::LBRACKET, "["},
						{mcf::Token::Type::RBRACKET, "]"},
						{mcf::Token::Type::COLON, ":"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::COMMA, ","},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 1. 리터럴
					"12345 \"hello, world!\"",
					{
						{mcf::Token::Type::INTEGER, "12345"},
						{mcf::Token::Type::STRING_UTF8, "\"hello, world!\""},
						{mcf::Token::Type::END_OF_FILE, "\0"},
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
						{mcf::Token::Type::KEYWORD_ENUM, "enum"},
						{mcf::Token::Type::KEYWORD_UNUSED, "unused"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 3. '.' 으로 시작하는 토큰
					"...",
					{
						{mcf::Token::Type::KEYWORD_VARIADIC, "..."},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 4. 매크로
					"#include <hello, world!>\n#include \"custom_file.hmcf\"",
					{
						{mcf::Token::Type::MACRO_IIBRARY_FILE_INCLUDE, "#include <hello, world!>"},
						{mcf::Token::Type::MACRO_PROJECT_FILE_INCLUDE, "#include \"custom_file.hmcf\""},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 5. 주석
					u8"// 한줄 주석입니다.\n/* 여러 줄을 주석\n 처리\n 할수 있습니다. */",
					{
						{mcf::Token::Type::COMMENT, u8"// 한줄 주석입니다."},
						{mcf::Token::Type::COMMENT_BLOCK, u8"/* 여러 줄을 주석\n 처리\n 할수 있습니다. */"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 6. 변수 관련 토큰
					"int32 foo = 1;uint8 boo = -7;",
					{
						{mcf::Token::Type::KEYWORD_INT32, "int32"},
						{mcf::Token::Type::IDENTIFIER, "foo"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::INTEGER, "1"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::KEYWORD_UINT8, "uint8"},
						{mcf::Token::Type::IDENTIFIER, "boo"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::MINUS, "-"},
						{mcf::Token::Type::INTEGER, "7"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 7. enum 관련 토큰
					"enum PRINT_RESULT : uint8{SUCCESS};enum PRINT_RESULT2 : int8{INVALID,ERROR1=1,ERROR2};enum PRINT_RESULT3 : int8{INVALID=0,ERROR1,ERROR2,COUNT,};",
					{
						{mcf::Token::Type::KEYWORD_ENUM, "enum"},
						{mcf::Token::Type::IDENTIFIER, "PRINT_RESULT"},
						{mcf::Token::Type::COLON, ":"},
						{mcf::Token::Type::KEYWORD_UINT8, "uint8"},
						{mcf::Token::Type::LBRACE, "{"},
						{mcf::Token::Type::IDENTIFIER, "SUCCESS"},
						{mcf::Token::Type::RBRACE, "}"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::KEYWORD_ENUM, "enum"},
						{mcf::Token::Type::IDENTIFIER, "PRINT_RESULT2"},
						{mcf::Token::Type::COLON, ":"},
						{mcf::Token::Type::KEYWORD_INT8, "int8"},
						{mcf::Token::Type::LBRACE, "{"},
						{mcf::Token::Type::IDENTIFIER, "INVALID"},
						{mcf::Token::Type::COMMA, ","},
						{mcf::Token::Type::IDENTIFIER, "ERROR1"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::INTEGER, "1"},
						{mcf::Token::Type::COMMA, ","},
						{mcf::Token::Type::IDENTIFIER, "ERROR2"},
						{mcf::Token::Type::RBRACE, "}"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::KEYWORD_ENUM, "enum"},
						{mcf::Token::Type::IDENTIFIER, "PRINT_RESULT3"},
						{mcf::Token::Type::COLON, ":"},
						{mcf::Token::Type::KEYWORD_INT8, "int8"},
						{mcf::Token::Type::LBRACE, "{"},
						{mcf::Token::Type::IDENTIFIER, "INVALID"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::INTEGER, "0"},
						{mcf::Token::Type::COMMA, ","},
						{mcf::Token::Type::IDENTIFIER, "ERROR1"},
						{mcf::Token::Type::COMMA, ","},
						{mcf::Token::Type::IDENTIFIER, "ERROR2"},
						{mcf::Token::Type::COMMA, ","},
						{mcf::Token::Type::IDENTIFIER, "COUNT"},
						{mcf::Token::Type::COMMA, ","},
						{mcf::Token::Type::RBRACE, "}"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 8. 함수 전방 선언 관련 토큰
					"const PRINT_RESULT Print(const utf8 format[], ...) const;",
					{
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::IDENTIFIER, "PRINT_RESULT"},
						{mcf::Token::Type::IDENTIFIER, "Print"},
						{mcf::Token::Type::LPAREN, "("},
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::KEYWORD_UTF8, "utf8"},
						{mcf::Token::Type::IDENTIFIER, "format"},
						{mcf::Token::Type::LBRACKET, "["},
						{mcf::Token::Type::RBRACKET, "]"},
						{mcf::Token::Type::COMMA, ","},
						{mcf::Token::Type::KEYWORD_VARIADIC, "..."},
						{mcf::Token::Type::RPAREN, ")"},
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 9. #include 메크로
					"#include <builtins> // include vector, string, print",
					{
						{mcf::Token::Type::MACRO_IIBRARY_FILE_INCLUDE, "#include <builtins>"},
						{mcf::Token::Type::COMMENT, "// include vector, string, print"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 10. 함수 선언 토큰
					"void main(void) { const utf8 str[] = \"Hello, World!\"; /* default string literal is static array of utf8 in mcf */ Print(\"%s\\n\", str); }",
					{
						{mcf::Token::Type::KEYWORD_VOID, "void"},
						{mcf::Token::Type::IDENTIFIER, "main"},
						{mcf::Token::Type::LPAREN, "("},
						{mcf::Token::Type::KEYWORD_VOID, "void"},
						{mcf::Token::Type::RPAREN, ")"},
						{mcf::Token::Type::LBRACE, "{"},
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::KEYWORD_UTF8, "utf8"},
						{mcf::Token::Type::IDENTIFIER, "str"},
						{mcf::Token::Type::LBRACKET, "["},
						{mcf::Token::Type::RBRACKET, "]"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::STRING_UTF8, "\"Hello, World!\""},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::COMMENT_BLOCK, "/* default string literal is static array of utf8 in mcf */"},
						{mcf::Token::Type::IDENTIFIER, "Print"},
						{mcf::Token::Type::LPAREN, "("},
						{mcf::Token::Type::STRING_UTF8, "\"%s\\n\""},
						{mcf::Token::Type::COMMA, ","},
						{mcf::Token::Type::IDENTIFIER, "str"},
						{mcf::Token::Type::RPAREN, ")"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::RBRACE, "}"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 11. 문자열
					"const utf8 str[] = \"Hello, World!\"; // default string literal is static array of utf8 in mcf",
					{
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::KEYWORD_UTF8, "utf8"},
						{mcf::Token::Type::IDENTIFIER, "str"},
						{mcf::Token::Type::LBRACKET, "["},
						{mcf::Token::Type::RBRACKET, "]"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::STRING_UTF8, "\"Hello, World!\""},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::COMMENT, "// default string literal is static array of utf8 in mcf"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 12. SLASH END_OF_FILE
					"/",
					{
						{mcf::Token::Type::SLASH, "/"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 13. 주석 END_OF_FILE
					"//",
					{
						{mcf::Token::Type::COMMENT, "//"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 14. 주석 블록
					"const /* utf8  */ int32 comment_block_test = 5;",
					{
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::COMMENT_BLOCK, "/* utf8  */"},
						{mcf::Token::Type::KEYWORD_INT32, "int32"},
						{mcf::Token::Type::IDENTIFIER, "comment_block_test"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::INTEGER, "5"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
				{ // 15. bool 변수 및 비교/논리 연산자
					"const bool foo = false; const bool boo = true; const bool jar = foo == boo; const bool bar = jar != boo; const bool tar = !jar;",
					{
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::KEYWORD_BOOL, "bool"},
						{mcf::Token::Type::IDENTIFIER, "foo"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::KEYWORD_FALSE, "false"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::KEYWORD_BOOL, "bool"},
						{mcf::Token::Type::IDENTIFIER, "boo"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::KEYWORD_TRUE, "true"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::KEYWORD_BOOL, "bool"},
						{mcf::Token::Type::IDENTIFIER, "jar"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::IDENTIFIER, "foo"},
						{mcf::Token::Type::EQUAL, "=="},
						{mcf::Token::Type::IDENTIFIER, "boo"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::KEYWORD_BOOL, "bool"},
						{mcf::Token::Type::IDENTIFIER, "bar"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::IDENTIFIER, "jar"},
						{mcf::Token::Type::NOT_EQUAL, "!="},
						{mcf::Token::Type::IDENTIFIER, "boo"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::KEYWORD_CONST, "const"},
						{mcf::Token::Type::KEYWORD_BOOL, "bool"},
						{mcf::Token::Type::IDENTIFIER, "tar"},
						{mcf::Token::Type::ASSIGN, "="},
						{mcf::Token::Type::BANG, "!"},
						{mcf::Token::Type::IDENTIFIER, "jar"},
						{mcf::Token::Type::SEMICOLON, ";"},
						{mcf::Token::Type::END_OF_FILE, "\0"},
					},
				},
			};
			constexpr const size_t testCaseCount = ARRAY_SIZE(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				const size_t expectedResultSize = testCases[i].ExpectedResults.size();
				mcf::Lexer::Object lexer(testCases[i].Input, false);

				std::vector<mcf::Token::Data>  actualTokens;
				mcf::Token::Data token = lexer.ReadNextToken();
				actualTokens.emplace_back(token);
				while (token.Type != mcf::Token::Type::END_OF_FILE || token.Type == mcf::Token::Type::INVALID)
				{
					token = lexer.ReadNextToken();
					actualTokens.emplace_back(token);
				}
				const size_t actualTokenCount = actualTokens.size();
				FATAL_ASSERT(expectedResultSize == actualTokenCount, u8"기대값의 갯수와 실제 생성된 토큰의 갯수가 같아야 합니다. 예상값=%zu, 실제값=%zu", expectedResultSize, actualTokenCount);

				for (size_t j = 0; j < expectedResultSize; j++)
				{
					FATAL_ASSERT(actualTokens[j].Type == testCases[i].ExpectedResults[j].Type, u8"tests[%zu-%zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
						i, j, mcf::Token::CONVERT_TYPE_TO_STRING(testCases[i].ExpectedResults[j].Type), mcf::Token::CONVERT_TYPE_TO_STRING(actualTokens[j].Type));

					FATAL_ASSERT(actualTokens[j].Literal == testCases[i].ExpectedResults[j].Literal, u8"tests[%zu-%zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
						i, j, testCases[i].ExpectedResults[j].Literal.c_str(), actualTokens[j].Literal.c_str());
				}
			}

			return true;
		});

		_names.emplace_back("./test/unittest/texts/test_file_read.txt");
		_tests.emplace_back([&]() {
			struct expected_result final
			{
				const mcf::Token::Type  Type;
				const char*				Literal;
			};

			const std::vector<expected_result>  expectedResults =
			{
					{mcf::Token::Type::END_OF_FILE, "\0"},
			};
			const size_t expectedResultSize = expectedResults.size();

			mcf::Lexer::Object lexer(_names.back().c_str(), true);

			std::vector<mcf::Token::Data>  actualTokens;
			mcf::Token::Data token = lexer.ReadNextToken();
			actualTokens.emplace_back(token);
			while (token.Type != mcf::Token::Type::END_OF_FILE || token.Type == mcf::Token::Type::INVALID)
			{
				token = lexer.ReadNextToken();
				actualTokens.emplace_back(token);
			}
			const size_t actualTokenCount = actualTokens.size();
			FATAL_ASSERT(expectedResultSize == actualTokenCount, u8"기대값의 갯수와 실제 생성된 토큰의 갯수가 같아야 합니다. 예상값=%zu, 실제값=%zu", expectedResultSize, actualTokenCount);

			for (size_t i = 0; i < actualTokenCount; i++)
			{
				FATAL_ASSERT(actualTokens[i].Type == expectedResults[i].Type, u8"tests[line: %zu, index: %zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
					actualTokens[i].Line, actualTokens[i].Index, mcf::Token::CONVERT_TYPE_TO_STRING(expectedResults[i].Type), mcf::Token::CONVERT_TYPE_TO_STRING(actualTokens[i].Type));

				FATAL_ASSERT(actualTokens[i].Literal == expectedResults[i].Literal, u8"tests[line: %zu, index: %zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
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