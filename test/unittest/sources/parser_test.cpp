#include <iostream>
#include "test/unittest/unittest.h"


UnitTest::ParserTest::ParserTest(void) noexcept
{
	_names.emplace_back(u8"0. include 매크로 명령문 테스트");
	_tests.emplace_back
	(
		[&]() -> bool
		{
			const struct TestCase
			{
				const std::string Input;
				const std::string Expected;
			} testCases[] =
			{
				{
					"#include <asm, \"kernel32.lib\">",
					"[IncludeLibrary: LT asm COMMA \"kernel32.lib\" GT]",
				}
			};
			constexpr const size_t testCaseCount = ARRAY_SIZE(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");

				const std::string actual = program.ConvertToString();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);
	_names.emplace_back(u8"1. typedef 명령문 테스트");
	_tests.emplace_back
	(
		[&]() -> bool
		{
			const struct TestCase
			{
				const std::string Input;
				const std::string Expected;
			} testCases[] =
			{
				{
					"typedef char: byte;",
					"[Typedef: <VariableSignature: <Identifier: char> COLON <TypeSignature: <Identifier: byte>>> SEMICOLON]",
				},
				{
					"typedef int32: byte[2];",
					"[Typedef: <VariableSignature: <Identifier: int32> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET <Integer: 2> RBRACKET>>> SEMICOLON]",
				},
				{
					"typedef int64: byte[6 + 2];",
					"[Typedef: <VariableSignature: <Identifier: int64> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET <Infix: <Integer: 6> PLUS <Integer: 2>> RBRACKET>>> SEMICOLON]",
				},
				{
					"typedef address: byte[4];",
					"[Typedef: <VariableSignature: <Identifier: address> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET <Integer: 4> RBRACKET>>> SEMICOLON]",
				},
				{
					"typedef bool: byte -> bind { false = 0, true = 1, };",
					std::string("[Typedef: <VariableSignature: <Identifier: bool> COLON <TypeSignature: <Identifier: byte>>> POINTING KEYWORD_BIND ") +
						"<MapInitializer: LBRACE " + 
							"<Identifier: false> ASSIGN <Integer: 0> COMMA " +
							"<Identifier: true> ASSIGN <Integer: 1> COMMA " +
						"RBRACE> " +
					"SEMICOLON]",
				},
			};
			constexpr const size_t testCaseCount = ARRAY_SIZE(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");

				const std::string actual = program.ConvertToString();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);
	_names.emplace_back(u8"2. extern 명령문 테스트");
	_tests.emplace_back
	(
		[&]() -> bool
		{
			const struct TestCase
			{
				const std::string Input;
				const std::string Expected;
			} testCases[] =
			{
				{
					"extern asm func printf(format: address, ...args) -> int32;",
					std::string("[Extern KEYWORD_ASM <FunctionSignature: <Identifier: printf> <FunctionParams: LPAREN ") + 
						"<VariableSignature: <Identifier: format> COLON <TypeSignature: <Identifier: address>>> COMMA " + 
						"<Variadic: <Identifier: args>> " +
					"RPAREN> POINTING <TypeSignature: <Identifier: int32>>> SEMICOLON]",
				}
			};
			constexpr const size_t testCaseCount = ARRAY_SIZE(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");

				const std::string actual = program.ConvertToString();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);
	_names.emplace_back(u8"3. let 명령문 테스트");
	_tests.emplace_back
	(
		[&]() -> bool
		{
			const struct TestCase
			{
				const std::string Input;
				const std::string Expected;
			} testCases[] =
			{
				{
					"let foo: byte = 0;",
					"[Let: <VariableSignature: <Identifier: foo> COLON <TypeSignature: <Identifier: byte>>> ASSIGN <Integer: 0> SEMICOLON]",
				},
				{
					"let arr: byte[] = { 0, 1, 2 };",
					std::string("[Let: <VariableSignature: <Identifier: arr> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET RBRACKET>>> ASSIGN ") +
					"<Initializer: LBRACE <Integer: 0> COMMA <Integer: 1> COMMA <Integer: 2> COMMA RBRACE> " +
					"SEMICOLON]",
				},
				{
					"let arr2: byte[5] = { 0 };",
					std::string("[Let: <VariableSignature: <Identifier: arr2> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET <Integer: 5> RBRACKET>>> ASSIGN ") +
					"<Initializer: LBRACE <Integer: 0> COMMA RBRACE> " +
					"SEMICOLON]",
				},
				{
					"let intVal: int32 = 10;",
					"[Let: <VariableSignature: <Identifier: intVal> COLON <TypeSignature: <Identifier: int32>>> ASSIGN <Integer: 10> SEMICOLON]",
				},
			};
			constexpr const size_t testCaseCount = ARRAY_SIZE(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");

				const std::string actual = program.ConvertToString();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);
	_names.emplace_back(u8"4. block 명령문 테스트");
	_tests.emplace_back
	(
		[&]() -> bool
		{
			const struct TestCase
			{
				const std::string Input;
				const std::string Expected;
			} testCases[] =
			{
				{
					"{ let a : byte = 0; let b : byte[2] = 1; }",
					std::string("[Block: LBRACE ") +
						"[Let: <VariableSignature: <Identifier: a> COLON <TypeSignature: <Identifier: byte>>> ASSIGN <Integer: 0> SEMICOLON] " +
						"[Let: <VariableSignature: <Identifier: b> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET <Integer: 2> RBRACKET>>> ASSIGN <Integer: 1> SEMICOLON] " +
					"RBRACE]"
				},
			};
			constexpr const size_t testCaseCount = ARRAY_SIZE(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");

				const std::string actual = program.ConvertToString();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);
	_names.emplace_back(u8"6. func 명령문 테스트");
	_tests.emplace_back
	(
		[&]() -> bool
		{
			const struct TestCase
			{
				const std::string Input;
				const std::string Expected;
			} testCases[] =
			{
				{
					"func boo(void) -> byte { return 0; }",
					std::string("[Func: ") +
						"<FunctionSignature: <Identifier: boo> <FunctionParams: KEYWORD_VOID> POINTING <TypeSignature: <Identifier: byte>>> " +
						"<Statements: LBRACE <Return: <Integer: 0>> RBRACE>" +
					"]"
				}
			};
			constexpr const size_t testCaseCount = ARRAY_SIZE(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");

				const std::string actual = program.ConvertToString();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);
}

bool UnitTest::ParserTest::CheckParserErrors(mcf::Parser::Object& parser) noexcept
{
	const size_t errorCount = parser.GetErrorCount();
	if (errorCount == 0)
	{
		return true;
	}

	for (mcf::Parser::ErrorInfo curr = parser.PopLastError(); curr.ID != mcf::Parser::ErrorID::SUCCESS; curr = parser.PopLastError())
	{
		printf("%s(%zu,%zu): error P%zu: %s\n", curr.Name.c_str(), curr.Line, curr.Index, mcf::ENUM_INDEX(curr.ID), curr.Message.c_str());
	}
	return false;
}
