#include <iostream>
#include <vector>
#include <string>


#include "../unittest.h"

namespace UnitTest
{
	LexerTest::LexerTest(void) noexcept
	{
		_names.emplace_back(u8"토큰 생성 테스트");
		_tests.emplace_back([&]() {
			const struct test_case
			{
				const std::string                   Input;
				const std::vector<mcf::Token::Data>  ExpectedResults;
			} testCases[] =
			{
				{ // 0. 연산자, 구분자, 및 블록 지정자
					"=+-*/<>&(){}[]:;,->",
					{
						TokenAssign,
						TokenPlus,
						TokenMinus,
						TokenAsterisk,
						TokenSlash,
						TokenLT,
						TokenGT,
						TokenAmpersand,
						TokenLParen,
						TokenRParen,
						TokenLBrace,
						TokenRBrace,
						TokenLBracket,
						TokenRBracket,
						TokenColon,
						TokenSemicolon,
						TokenComma,
						TokenPointing,
						TokenEOF,
					},
				},
				{ // 1. 리터럴
					"12345 \"hello, world!\"",
					{
						TokenInteger("12345"),
						TokenString("\"hello, world!\""),
						TokenEOF,
					},
				},
				{ // 2. 식별자 키워드
					"void unused",
					{
						TokenVoid,
						TokenUnused,
						TokenEOF,
					},
				},
				{ // 3. '.' 으로 시작하는 토큰
					"...",
					{
						TokenVariadic,
						TokenEOF,
					},
				},
				{ // 4. 매크로
					"#include <asm, \"kernel32.lib\">",
					{
						TokenInclude,
						TokenLT,
						TokenASM,
						TokenComma,
						TokenString("\"kernel32.lib\""),
						TokenGT,
						TokenEOF,
					},
				},
				{ // 5. 주석
					u8"// 한줄 주석입니다.\n/* 여러 줄을 주석\n 처리\n 할수 있습니다. */",
					{
						TokenComment(u8"// 한줄 주석입니다."),
						TokenCommentBlock(u8"/* 여러 줄을 주석\n 처리\n 할수 있습니다. */"),
						TokenEOF,
					},
				},
				{ // 6. 변수 관련 토큰
					"let foo: byte = 1;let boo: byte = -7;",
					{
						TokenLet,
						TokenIdentifier("foo"),
						TokenColon,
						TokenByte,
						TokenAssign,
						TokenInteger("1"),
						TokenSemicolon,
						TokenLet,
						TokenIdentifier("boo"),
						TokenColon,
						TokenByte,
						TokenAssign,
						TokenMinus,
						TokenInteger("7"),
						TokenSemicolon,
						TokenEOF,
					},
				},
				{ // 7. extern 함수 관련 토큰
					"extern func printf(format: byte[4], ...args) -> byte[4];",
					{
						TokenExtern,
						TokenFunc,
						TokenIdentifier("printf"),
						TokenLParen,
						TokenIdentifier("format"),
						TokenColon,
						TokenByte,
						TokenLBracket,
						TokenInteger("4"),
						TokenRBracket,
						TokenComma,
						TokenVariadic,
						TokenIdentifier("args"),
						TokenRParen,
						TokenPointing,
						TokenByte,
						TokenLBracket,
						TokenInteger("4"),
						TokenRBracket,
						TokenSemicolon,
						TokenEOF,
					},
				},
				{ // 8. 메인 선언 토큰
					"main(void) -> void { printf(&\"Hello, World!\\n\"); }",
					{
						TokenMain,
						TokenLParen,
						TokenVoid,
						TokenRParen,
						TokenPointing,
						TokenVoid,
						TokenLBrace,
						TokenIdentifier("printf"),
						TokenLParen,
						TokenAmpersand,
						TokenString("\"Hello, World!\\n\""),
						TokenRParen,
						TokenSemicolon,
						TokenRBrace,
						TokenEOF,
					},
				},
				{ // 9. 문자열
					"let str: byte[] = \"Hello, World!\"; // default string literal is static array of utf8 in mcf",
					{
						TokenLet,
						TokenIdentifier("str"),
						TokenColon,
						TokenByte,
						TokenLBracket,
						TokenRBracket,
						TokenAssign,
						TokenString("\"Hello, World!\""),
						TokenSemicolon,
						TokenComment("// default string literal is static array of utf8 in mcf"),
						TokenEOF,
					},
				},
				{ // 10. SLASH END_OF_FILE
					"/",
					{
						TokenSlash,
						TokenEOF,
					},
				},
				{ // 11. 주석 END_OF_FILE
					"//",
					{
						TokenComment("//"),
						TokenEOF,
					},
				},
				{ // 12. 주석 블록
					"let str: byte[] /* utf8 */ = \"Hello, World!\"; // default string literal is static array of utf8 in mcf",
					{
						TokenLet,
						TokenIdentifier("str"),
						TokenColon,
						TokenByte,
						TokenLBracket,
						TokenRBracket,
						TokenCommentBlock("/* utf8 */"),
						TokenAssign,
						TokenString("\"Hello, World!\""),
						TokenSemicolon,
						TokenComment("// default string literal is static array of utf8 in mcf"),
						TokenEOF,
					},
				},
				//{ // 13. bool 변수 및 비교/논리 연산자
				//	"const bool foo = false; const bool boo = true; const bool jar = foo == boo; const bool bar = jar != boo; const bool tar = !jar;",
				//	{
				//		{mcf::Token::Type::IDENTIFIER, "const"},
				//		{mcf::Token::Type::KEYWORD_BOOL, "bool"},
				//		{mcf::Token::Type::IDENTIFIER, "foo"},
				//		token_assign,
				//		{mcf::Token::Type::KEYWORD_FALSE, "false"},
				//		token_semicolon,
				//		{mcf::Token::Type::IDENTIFIER, "const"},
				//		{mcf::Token::Type::KEYWORD_BOOL, "bool"},
				//		{mcf::Token::Type::IDENTIFIER, "boo"},
				//		token_assign,
				//		{mcf::Token::Type::KEYWORD_TRUE, "true"},
				//		token_semicolon,
				//		{mcf::Token::Type::IDENTIFIER, "const"},
				//		{mcf::Token::Type::KEYWORD_BOOL, "bool"},
				//		{mcf::Token::Type::IDENTIFIER, "jar"},
				//		token_assign,
				//		{mcf::Token::Type::IDENTIFIER, "foo"},
				//		{mcf::Token::Type::EQUAL, "=="},
				//		{mcf::Token::Type::IDENTIFIER, "boo"},
				//		token_semicolon,
				//		{mcf::Token::Type::IDENTIFIER, "const"},
				//		{mcf::Token::Type::KEYWORD_BOOL, "bool"},
				//		{mcf::Token::Type::IDENTIFIER, "bar"},
				//		token_assign,
				//		{mcf::Token::Type::IDENTIFIER, "jar"},
				//		{mcf::Token::Type::NOT_EQUAL, "!="},
				//		{mcf::Token::Type::IDENTIFIER, "boo"},
				//		token_semicolon,
				//		{mcf::Token::Type::IDENTIFIER, "const"},
				//		{mcf::Token::Type::KEYWORD_BOOL, "bool"},
				//		{mcf::Token::Type::IDENTIFIER, "tar"},
				//		token_assign,
				//		{mcf::Token::Type::BANG, "!"},
				//		{mcf::Token::Type::IDENTIFIER, "jar"},
				//		token_semicolon,
				//		token_eof,
				//	},
				//},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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

		_names.emplace_back(u8"파일 읽기 및 토큰 생성 테스트");
		_tests.emplace_back([&]() {
			const std::vector<mcf::Token::Data>  expectedResults =
			{
				// #include <asm, "kernel32.lib">
				TokenInclude,
				TokenLT,
				TokenASM,
				TokenComma,
				TokenString("\"kernel32.lib\""),
				TokenGT,

				// typedef int32: dword;
				TokenTypedef,
				TokenIdentifier("int32"),
				TokenColon,
				TokenDword,
				TokenSemicolon,

				// typedef uint32: unsigned dword;
				TokenTypedef,
				TokenIdentifier("uint32"),
				TokenColon,
				TokenUnsigned,
				TokenDword,
				TokenSemicolon,

				// typedef address: unsigned qword;
				TokenTypedef,
				TokenIdentifier("address"),
				TokenColon,
				TokenUnsigned,
				TokenQword,
				TokenSemicolon,

				/*
				* typedef bool: byte -> bind
				* {
				*	false = 0,
				*	true = 1,
				* };
				*/
				TokenTypedef,
				TokenIdentifier("bool"),
				TokenColon,
				TokenByte,
				TokenPointing,
				TokenBind,
				TokenLBrace,
				TokenIdentifier("false"),
				TokenAssign,
				TokenInteger("0"),
				TokenComma,
				TokenIdentifier("true"),
				TokenAssign,
				TokenInteger("1"),
				TokenComma,
				TokenRBrace,
				TokenSemicolon,

				// #include <asm, "libcmt.lib">
				TokenInclude,
				TokenLT,
				TokenASM,
				TokenComma,
				TokenString("\"libcmt.lib\""),
				TokenGT,

				// extern func printf(format: unsigned qword, ...args) -> int32;
				TokenExtern,
				TokenFunc,
				TokenIdentifier("printf"),
				TokenLParen,
				TokenIdentifier("format"),
				TokenColon,
				TokenUnsigned,
				TokenQword,
				TokenComma,
				TokenVariadic,
				TokenIdentifier("args"),
				TokenRParen,
				TokenPointing,
				TokenIdentifier("int32"),
				TokenSemicolon,

				// let foo: byte = 0;
				TokenLet,
				TokenIdentifier("foo"),
				TokenColon,
				TokenByte,
				TokenAssign,
				TokenInteger("0"),
				TokenSemicolon,

				// func boo(void) -> byte { return 0; }
				TokenFunc,
				TokenIdentifier("boo"),
				TokenLParen,
				TokenVoid,
				TokenRParen,
				TokenPointing,
				TokenByte,
				TokenLBrace,
				TokenReturn,
				TokenInteger("0"),
				TokenSemicolon,
				TokenRBrace,

				// let arr: byte[] = { 0, 1, 2 };
				TokenLet,
				TokenIdentifier("arr"),
				TokenColon,
				TokenByte,
				TokenLBracket,
				TokenRBracket,
				TokenAssign,
				TokenLBrace,
				TokenInteger("0"),
				TokenComma,
				TokenInteger("1"),
				TokenComma,
				TokenInteger("2"),
				TokenRBrace,
				TokenSemicolon,

				// let arr2: byte[5] = { 0 };
				TokenLet,
				TokenIdentifier("arr2"),
				TokenColon,
				TokenByte,
				TokenLBracket,
				TokenInteger("5"),
				TokenRBracket,
				TokenAssign,
				TokenLBrace,
				TokenInteger("0"),
				TokenRBrace,
				TokenSemicolon,

				// let intVal: int32 = 10;
				TokenLet,
				TokenIdentifier("intVal"),
				TokenColon,
				TokenIdentifier("int32"),
				TokenAssign,
				TokenInteger("10"),
				TokenSemicolon,
			
				/*
				* main(void) -> void
				* {
				*	unused(foo, boo, arr, arr2);
				*	let message: byte[] = "Hello, World! Value=%d\n";
				*	printf(message as unsigned qword, intVal);
				* }
				*/
				TokenMain,
				TokenLParen,
				TokenVoid,
				TokenRParen,
				TokenPointing,
				TokenVoid,
				TokenLBrace,
				TokenUnused,
				TokenLParen,
				TokenIdentifier("foo"),
				TokenComma,
				TokenIdentifier("boo"),
				TokenComma,
				TokenIdentifier("arr"),
				TokenComma,
				TokenIdentifier("arr2"),
				TokenRParen,
				TokenSemicolon,
				TokenLet,
				TokenIdentifier("message"),
				TokenColon,
				TokenByte,
				TokenLBracket,
				TokenRBracket,
				TokenAssign,
				TokenString("\"Hello, World! Value=%d\\n\""),
				TokenSemicolon,
				TokenIdentifier("printf"),
				TokenLParen,
				TokenIdentifier("message"),
				TokenAs,
				TokenUnsigned,
				TokenQword,
				TokenComma,
				TokenIdentifier("intVal"),
				TokenRParen,
				TokenSemicolon,
				TokenRBrace,

				// EOF
				TokenEOF,
			};
			const size_t expectedResultSize = expectedResults.size();

			mcf::Lexer::Object lexer("./test/unittest/texts/test_file_read.txt", true);

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
					actualTokens[i].Line, actualTokens[i].Index, expectedResults[i].Literal.c_str(), actualTokens[i].Literal.c_str());
			}

			return true;
			});
	}
}