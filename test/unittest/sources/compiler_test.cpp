#include <iostream>
#include "../unittest.h"

UnitTest::CompilerTest::CompilerTest(void) noexcept
{
	_names.emplace_back( u8".data 컴파일 테스트" );
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
					"typedef int32: dword;",
					".data\n"
					"int32 typedef dword",
				},
				{
					"typedef uint32: unsigned dword;",
					".data\n"
					"uint32 typedef dword",
				},
				{
					"typedef address: unsigned qword;",
					".data\n"
					"address typedef qword",
				},
				{
					"typedef bool: byte;"
					"let false: bool = 0;"
					"let true: bool = 1;",
					".data\n"
					"bool typedef byte\n"
					"false bool 0\n"
					"true bool 1",
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);
			for (size_t i = 0; i < testCaseCount; i++)
			{ 
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");

				mcf::Object::TypeInfo byteType = mcf::Object::TypeInfo::MakePrimitive(false, "byte", 1);
				mcf::Object::TypeInfo wordType = mcf::Object::TypeInfo::MakePrimitive(false, "word", 2);
				mcf::Object::TypeInfo dwordType = mcf::Object::TypeInfo::MakePrimitive(false, "dword", 4);
				mcf::Object::TypeInfo qwordType = mcf::Object::TypeInfo::MakePrimitive(false, "qword", 8);

				mcf::Object::ScopeTree scopeTree;
				scopeTree.Global.DefineType(byteType.Name, byteType);
				scopeTree.Global.DefineType(wordType.Name, wordType);
				scopeTree.Global.DefineType(dwordType.Name, dwordType);
				scopeTree.Global.DefineType(qwordType.Name, qwordType);

				mcf::Evaluator::Object evaluator;
				mcf::IR::Pointer irCodes = evaluator.Eval(&program, &scopeTree.Global);
				FATAL_ASSERT( irCodes.get() != nullptr, u8"irCodes이 nullptr이면 안됩니다.");
				FATAL_ASSERT( irCodes->GetType() == mcf::IR::Type::PROGRAM, u8"irCodes이 IR::Type::PROGRAM이 아닙니다.");

				mcf::Compiler::Object compiler;
				mcf::ASM::PointerVector generatedCodes = compiler.GenerateCodes(irCodes, &scopeTree);
				const std::string actual = object->Inspect();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);
}

bool UnitTest::CompilerTest::CheckParserErrors(mcf::Parser::Object& parser) noexcept
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
