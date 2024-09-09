#include <iostream>
#include "../unittest.h"

UnitTest::ParserTest::ParserTest(void) noexcept
{
	_names.emplace_back(u8"include 매크로 명령문 테스트");
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
					"[IncludeLibrary: LT KEYWORD_ASM COMMA \"kernel32.lib\" GT]",
				}
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back(u8"typedef 명령문 테스트");
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
					"typedef int32: dword;",
					"[Typedef: <VariableSignature: <Identifier: int32> COLON <TypeSignature: <Identifier: dword>>> SEMICOLON]",
				},
				{
					"typedef int64: qword;",
					"[Typedef: <VariableSignature: <Identifier: int64> COLON <TypeSignature: <Identifier: qword>>> SEMICOLON]",
				},
				{
					"typedef address: unsigned qword;",
					"[Typedef: <VariableSignature: <Identifier: address> COLON <TypeSignature: KEYWORD_UNSIGNED <Identifier: qword>>> SEMICOLON]",
				},
				{
					"typedef bool: byte;"
					"let false: bool = 0;"
					"let true: bool = 1;",
					"[Typedef: <VariableSignature: <Identifier: bool> COLON <TypeSignature: <Identifier: byte>>> SEMICOLON]\n"
					"[Let: <VariableSignature: <Identifier: false> COLON <TypeSignature: <Identifier: bool>>> ASSIGN <Integer: 0> SEMICOLON]\n"
					"[Let: <VariableSignature: <Identifier: true> COLON <TypeSignature: <Identifier: bool>>> ASSIGN <Integer: 1> SEMICOLON]",
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back(u8"extern 명령문 테스트");
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
					"extern func printf(format: unsigned qword, ...args) -> int32;",
					"[Extern <FunctionSignature: <Identifier: printf> <FunctionParams: LPAREN "
						"<VariableSignature: <Identifier: format> COLON <TypeSignature: KEYWORD_UNSIGNED <Identifier: qword>>> COMMA "
						"<Variadic: <Identifier: args>> "
					"RPAREN> POINTING <TypeSignature: <Identifier: int32>>> SEMICOLON]",
				}
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back(u8"let 명령문 테스트");
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
					"let foo: byte = -5;",
					"[Let: <VariableSignature: <Identifier: foo> COLON <TypeSignature: <Identifier: byte>>> ASSIGN <Prefix: MINUS <Integer: 5>> SEMICOLON]",
				},
				{
					"let arr: byte[] = { 0, 1, 2 };",
					"[Let: <VariableSignature: <Identifier: arr> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET RBRACKET>>> ASSIGN "
						"<Initializer: LBRACE <Integer: 0> COMMA <Integer: 1> COMMA <Integer: 2> COMMA RBRACE> "
					"SEMICOLON]",
				},
				{
					"let arr2: byte[5] = { 0 };",
					"[Let: <VariableSignature: <Identifier: arr2> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET <Integer: 5> RBRACKET>>> ASSIGN "
						"<Initializer: LBRACE <Integer: 0> COMMA RBRACE> "
					"SEMICOLON]",
				},
				{
					"let intVal: int32 = 10;",
					"[Let: <VariableSignature: <Identifier: intVal> COLON <TypeSignature: <Identifier: int32>>> ASSIGN <Integer: 10> SEMICOLON]",
				},
				{
					"let intVal: int32 = 10 * (5 + 5);",
					"[Let: <VariableSignature: <Identifier: intVal> COLON <TypeSignature: <Identifier: int32>>> ASSIGN "
						"<Infix: <Integer: 10> ASTERISK <Group: <Infix: <Integer: 5> PLUS <Integer: 5>>>> "
					"SEMICOLON]",
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back(u8"block 명령문 테스트");
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
					"{ let a : byte = 0; let b : byte[2] = 1; let c : bool = !false; }",
					"[Block: LBRACE "
						"[Let: <VariableSignature: <Identifier: a> COLON <TypeSignature: <Identifier: byte>>> ASSIGN <Integer: 0> SEMICOLON] "
						"[Let: <VariableSignature: <Identifier: b> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET <Integer: 2> RBRACKET>>> ASSIGN <Integer: 1> SEMICOLON] "
						"[Let: <VariableSignature: <Identifier: c> COLON <TypeSignature: <Identifier: bool>>> ASSIGN <Prefix: BANG <Identifier: false>> SEMICOLON] "
					"RBRACE]"
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back(u8"return 명령문 테스트");
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
					"return 0;",
					"[Return: <Integer: 0> SEMICOLON]"
				},
				{
					"return 2 == 2;",
					"[Return: <Infix: <Integer: 2> EQUAL <Integer: 2>> SEMICOLON]"
				},
				{
					"return 2 != 2;",
					"[Return: <Infix: <Integer: 2> NOT_EQUAL <Integer: 2>> SEMICOLON]"
				},
				{
					"return 2 < 2;",
					"[Return: <Infix: <Integer: 2> LT <Integer: 2>> SEMICOLON]"
				},
				{
					"return 2 > 2;",
					"[Return: <Infix: <Integer: 2> GT <Integer: 2>> SEMICOLON]"
				},
				{
					"return 2 == 2 < 2 == 2;",
					"[Return: <Infix: <Infix: <Integer: 2> EQUAL <Infix: <Integer: 2> LT <Integer: 2>>> EQUAL <Integer: 2>> SEMICOLON]"
				},
				{
					"return 2 + 2;",
					"[Return: <Infix: <Integer: 2> PLUS <Integer: 2>> SEMICOLON]"
				},
				{
					"return 2 - 2;",
					"[Return: <Infix: <Integer: 2> MINUS <Integer: 2>> SEMICOLON]"
				},
				{
					"return 2 * 2;",
					"[Return: <Infix: <Integer: 2> ASTERISK <Integer: 2>> SEMICOLON]"
				},
				{
					"return 2 / 2;",
					"[Return: <Infix: <Integer: 2> SLASH <Integer: 2>> SEMICOLON]"
				},
				{
					"return 2 == 2 < 2 + 2;",
					"[Return: <Infix: <Integer: 2> EQUAL <Infix: <Integer: 2> LT <Infix: <Integer: 2> PLUS <Integer: 2>>>> SEMICOLON]"
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back(u8"func 명령문 테스트");
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
					"[Func: "
						"<FunctionSignature: <Identifier: boo> <FunctionParams: LPAREN KEYWORD_VOID RPAREN> POINTING <TypeSignature: <Identifier: byte>>> "
						"[Block: LBRACE [Return: <Integer: 0> SEMICOLON] RBRACE]"
					"]"
				},
				{
					"func boo(void) -> byte { let a : byte = 2; return a + 5 * 3; }",
					"[Func: "
						"<FunctionSignature: <Identifier: boo> <FunctionParams: LPAREN KEYWORD_VOID RPAREN> POINTING <TypeSignature: <Identifier: byte>>> "
						"[Block: LBRACE "
							"[Let: <VariableSignature: <Identifier: a> COLON <TypeSignature: <Identifier: byte>>> ASSIGN <Integer: 2> SEMICOLON] "
							"[Return: <Infix: <Identifier: a> PLUS <Infix: <Integer: 5> ASTERISK <Integer: 3>>> SEMICOLON] "
						"RBRACE]"
					"]"
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back(u8"main 명령문 테스트");
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
					"main(void) -> byte { let foo: byte = 5; return 0; }",
					"[Main: "
						"<FunctionParams: LPAREN KEYWORD_VOID RPAREN> POINTING <TypeSignature: <Identifier: byte>> "
						"[Block: LBRACE "
							"[Let: <VariableSignature: <Identifier: foo> COLON <TypeSignature: <Identifier: byte>>> ASSIGN <Integer: 5> SEMICOLON] "
							"[Return: <Integer: 0> SEMICOLON] "
						"RBRACE]"
					"]"
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back(u8"expression 명령문 테스트");
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
					"foo;",
					"[Expression: <Identifier: foo> SEMICOLON]"
				},
				{
					"foo + boo;",
					"[Expression: <Infix: <Identifier: foo> PLUS <Identifier: boo>> SEMICOLON]"
				},
				{
					"foo + boo * 5;",
					"[Expression: <Infix: <Identifier: foo> PLUS <Infix: <Identifier: boo> ASTERISK <Integer: 5>>> SEMICOLON]"
				},
				{
					"boo();",
					"[Expression: <Call: <Identifier: boo> LPAREN RPAREN> SEMICOLON]"
				},
				{
					"printf(&message, intVal);",
					"[Expression: <Call: <Identifier: printf> LPAREN <Prefix: AMPERSAND <Identifier: message>> COMMA <Identifier: intVal> COMMA RPAREN> SEMICOLON]"
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back(u8"Unused 명령문 테스트");
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
					"unused(foo, boo, arr, arr2);",
					"[Unused: LPAREN <Identifier: foo> COMMA <Identifier: boo> COMMA <Identifier: arr> COMMA <Identifier: arr2> COMMA RPAREN SEMICOLON]"
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

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
	_names.emplace_back( u8"파일 파싱 테스트" );
	_tests.emplace_back
	(
		[&]() -> bool
		{
			std::string expectedResult =
				"[IncludeLibrary: LT KEYWORD_ASM COMMA \"kernel32.lib\" GT]\n"
				"[Typedef: <VariableSignature: <Identifier: int32> COLON <TypeSignature: <Identifier: dword>>> SEMICOLON]\n"
				"[Typedef: <VariableSignature: <Identifier: uint32> COLON <TypeSignature: KEYWORD_UNSIGNED <Identifier: dword>>> SEMICOLON]\n"
				"[Typedef: <VariableSignature: <Identifier: address> COLON <TypeSignature: KEYWORD_UNSIGNED <Identifier: qword>>> SEMICOLON]\n"
				"[Typedef: <VariableSignature: <Identifier: bool> COLON <TypeSignature: <Identifier: byte>>> SEMICOLON]\n"
				"[Let: <VariableSignature: <Identifier: false> COLON <TypeSignature: <Identifier: bool>>> ASSIGN <Integer: 0> SEMICOLON]\n"
				"[Let: <VariableSignature: <Identifier: true> COLON <TypeSignature: <Identifier: bool>>> ASSIGN <Integer: 1> SEMICOLON]\n"
				"[IncludeLibrary: LT KEYWORD_ASM COMMA \"libcmt.lib\" GT]\n"
				"[Extern <FunctionSignature: <Identifier: printf> <FunctionParams: LPAREN "
					"<VariableSignature: <Identifier: format> COLON <TypeSignature: KEYWORD_UNSIGNED <Identifier: qword>>> COMMA "
					"<Variadic: <Identifier: args>> "
				"RPAREN> POINTING <TypeSignature: <Identifier: int32>>> SEMICOLON]\n"
				"[Let: <VariableSignature: <Identifier: foo> COLON <TypeSignature: <Identifier: byte>>> ASSIGN <Integer: 0> SEMICOLON]\n"
				"[Func: "
					"<FunctionSignature: <Identifier: boo> <FunctionParams: LPAREN KEYWORD_VOID RPAREN> POINTING <TypeSignature: <Identifier: byte>>> "
					"[Block: LBRACE [Return: <Integer: 0> SEMICOLON] RBRACE]"
				"]\n"
				"[Let: <VariableSignature: <Identifier: arr> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET RBRACKET>>> ASSIGN "
					"<Initializer: LBRACE <Integer: 0> COMMA <Integer: 1> COMMA <Integer: 2> COMMA RBRACE> "
				"SEMICOLON]\n"
				"[Let: <VariableSignature: <Identifier: arr2> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET <Integer: 5> RBRACKET>>> ASSIGN "
					"<Initializer: LBRACE <Integer: 0> COMMA RBRACE> "
				"SEMICOLON]\n"
				"[Let: <VariableSignature: <Identifier: intVal> COLON <TypeSignature: <Identifier: int32>>> ASSIGN <Integer: 10> SEMICOLON]\n"
				"[Main: <FunctionParams: LPAREN KEYWORD_VOID RPAREN> POINTING KEYWORD_VOID "
				"[Block: LBRACE "
					"[Unused: LPAREN <Identifier: foo> COMMA <Identifier: arr> COMMA <Identifier: arr2> COMMA RPAREN SEMICOLON] "
					"[Let: <VariableSignature: <Identifier: message> COLON <TypeSignature: <Index: <Identifier: byte> LBRACKET RBRACKET>>> ASSIGN <String: \"Hello, World! Value=%d\\n\"> SEMICOLON] "
					"[Expression: <Call: <Identifier: printf> LPAREN <As: <Identifier: message> KEYWORD_AS <TypeSignature: KEYWORD_UNSIGNED <Identifier: qword>>> COMMA <Identifier: intVal> COMMA RPAREN> SEMICOLON] "
				"RBRACE]]"
				;
			const size_t expectedResultLength = expectedResult.size();
			mcf::Parser::Object parser("./test/unittest/texts/test_file_read.txt", true);
			mcf::AST::Program program;
			parser.ParseProgram(program);
			FATAL_ASSERT(CheckParserErrors( parser ), u8"파싱에 실패 하였습니다.");

			const std::string actual = program.ConvertToString();
			const size_t actualLength = actual.size();
			size_t lineNumber = 1;
			size_t position = 0;
			for (size_t i = 0; i < actualLength; i++)
			{
				FATAL_ASSERT(i < expectedResultLength, u8"Expected Result의 길이가 Actual Result의 길이보다 작습니다. ActualResultLength=%zu, ExpectedResultLength=%zu", actualLength, expectedResultLength);
				FATAL_ASSERT(expectedResult[i] == actual[i], u8"Expected[%c]가 Actual[%c]와 다릅니다. LineNumber=%zu, PositionInLine=%zu\nActualResult:\n%s", expectedResult[i], actual[i], lineNumber, position, actual.c_str());

				position++;
				if (expectedResult[i] == '\n')
				{
					lineNumber++;
					position = 0;;
				}
			}
			FATAL_ASSERT(actualLength == expectedResultLength, u8"Actual Result의 길이가 Expected Result의 길이보다 작습니다. ActualResultLength=%zu, ExpectedResultLength=%zu", actualLength, expectedResultLength);
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
