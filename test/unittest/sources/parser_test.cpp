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
				FATAL_ASSERT(actual == testCases[i].Expected, "expected=`%s`, actual=`%s`", testCases[i].Expected.c_str(), actual.c_str());
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
				/*{
					"typedef int64: byte[6 + 2];",
					"[Typedef: <VariableSignature: <Identifier: int64> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET <Infix: <Integer: 6> PLUS <Integer: 2>> RBRACKET>>> SEMICOLON]",
				},*/
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
						"RBRACE>"
					+" SEMICOLON]",
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
				FATAL_ASSERT(actual == testCases[i].Expected, "\nexpected=\t`%s`\nactual=\t\t`%s`", testCases[i].Expected.c_str(), actual.c_str());
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
