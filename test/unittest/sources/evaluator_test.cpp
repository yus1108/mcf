﻿#include <iostream>
#include "../unittest.h"

UnitTest::EvaluatorTest::EvaluatorTest(void) noexcept
{
	_names.emplace_back( u8"#include 평가 테스트" );
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
				{"#include <asm, \"kernel32.lib\">", "includelib \"kernel32.lib\""},
				{"#include <asm, \"libcmt.lib\">", "includelib \"libcmt.lib\""},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);

			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");
				mcf::Evaluator::Object evaluator;
				mcf::Object::Pointer object = evaluator.Eval(&program);

				FATAL_ASSERT(object.get() != nullptr, u8"object가 nullptr이면 안됩니다.");
				const std::string actual = object->Inspect();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);
	_names.emplace_back(u8"extern 평가 테스트");
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
					"extern func printf(format: unsigned qword, ...args) -> dword;",
					"printf PROTO : unsigned qword, VARARG",
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE( testCases );
			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");
				mcf::Evaluator::Object evaluator
				( 
					{
						mcf::Evaluator::TypeInfo::MakePrimitive("byte"),
						mcf::Evaluator::TypeInfo::MakePrimitive("word"),
						mcf::Evaluator::TypeInfo::MakePrimitive("dword"),
						mcf::Evaluator::TypeInfo::MakePrimitive("qword"),
					}
				);
				mcf::Object::Pointer object = evaluator.Eval(&program);

				FATAL_ASSERT(object.get() != nullptr, u8"object가 nullptr이면 안됩니다.");
				const std::string actual = object->Inspect();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());

				//const size_t externalFunctionsCount = testCases[i].ExpectedExternalFunctions.size();
				//for (size_t j = 0; j < externalFunctionsCount; ++j)
				//{
				//	const mcf::Evaluator::FunctionInfo actualFunctionInfo = evaluator.FindFunctionInfo(testCases[i].ExpectedExternalFunctions[j].GetName());
				//	FATAL_ASSERT(actualFunctionInfo == testCases[i].ExpectedExternalFunctions[j], "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s",
				//		i, testCases[i].Input.c_str(), testCases[i].ExpectedExternalFunctions[j].ConvertToString().c_str(), actualFunctionInfo.ConvertToString().c_str());
				//}
			}
			return true;
		}
	);
}

bool UnitTest::EvaluatorTest::CheckParserErrors(mcf::Parser::Object& parser) noexcept
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