#include <iostream>
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
				mcf::IR::Pointer object = evaluator.Eval(&program, nullptr);

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
				{
					"extern func printf(format: unsigned qword, ...args) -> dword[5];",
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
				mcf::Evaluator::Object evaluator;
				mcf::Object::ScopeTree scopeTree;
				scopeTree.Global.DefineType("byte", mcf::Object::TypeInfo::MakePrimitive("byte", 1));
				scopeTree.Global.DefineType("word", mcf::Object::TypeInfo::MakePrimitive("word", 2));
				scopeTree.Global.DefineType("dword", mcf::Object::TypeInfo::MakePrimitive("dword", 4));
				scopeTree.Global.DefineType("qword", mcf::Object::TypeInfo::MakePrimitive("qword", 8));
				mcf::IR::Pointer object = evaluator.Eval(&program, &scopeTree.Global);

				FATAL_ASSERT(object.get() != nullptr, u8"object가 nullptr이면 안됩니다.");
				const std::string actual = object->Inspect();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());

			}
			return true;
		}
	);
	_names.emplace_back(u8"let(GlobalVariableIdentifier) 평가 테스트");
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
					"foo byte 0",
				},
				{
					"let arr: byte[] = { 0, 1, 2 };",
					"arr byte[0] { 0, 1, 2, }",
				},
				{
					"let arr2: byte[5] = { 0 };",
					"arr2 byte[5] { 0, }",
				},
				{
					"let intVal: unsigned dword = 10;",
					"intVal unsigned dword 10",
				},
				{
					"let unInit: qword;",
					"unInit qword ?",
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);
			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");
				mcf::Evaluator::Object evaluator;
				mcf::Object::ScopeTree scopeTree;
				scopeTree.Global.DefineType("byte", mcf::Object::TypeInfo::MakePrimitive("byte", 1));
				scopeTree.Global.DefineType("word", mcf::Object::TypeInfo::MakePrimitive("word", 2));
				scopeTree.Global.DefineType("dword", mcf::Object::TypeInfo::MakePrimitive("dword", 4));
				scopeTree.Global.DefineType("qword", mcf::Object::TypeInfo::MakePrimitive("qword", 8));
				mcf::IR::Pointer object = evaluator.Eval(&program, &scopeTree.Global);

				FATAL_ASSERT(object.get() != nullptr, u8"object가 nullptr이면 안됩니다.");
				const std::string actual = object->Inspect();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());

			}
			return true;
		}
	);
	_names.emplace_back( u8"func 평가 테스트" );
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
					"func foo(void) -> void {}",
					"foo proc\n"
						"\tpush rbp\n"
						"\tpop rbp\n"
						"\tret\n"
					"foo endp\n",
				},
				{
					"func joo(param1: dword) -> void { unused(param1); }",
					"joo proc\n"
						"\tpush rbp\n"
						"\tmov qword ptr [rsp + 16], rcx\n" // var1 = 15;
						"\tpop rbp\n"
						"\tret\n"
					"joo endp\n",
				},
				{
					"func joo(void) -> void { let var1: dword = 15; unused(var1); }",
					"joo proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 16\n"
						"\tmov dword ptr [rsp + 0], 15\n" // var1 = 15;
						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"joo endp\n",
				},
				{
					"func boo(void) -> byte { return 0; }",
					"boo proc\n"
						"\tpush rbp\n"
						"\tmov return byte 0\n"
						"\tpop rbp\n"
						"\tret\n"
					"boo endp\n",
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);
			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");
				mcf::Evaluator::Object evaluator;
				mcf::Object::ScopeTree scopeTree;
				scopeTree.Global.DefineType("byte", mcf::Object::TypeInfo::MakePrimitive("byte", 1));
				scopeTree.Global.DefineType("word", mcf::Object::TypeInfo::MakePrimitive("word", 2));
				scopeTree.Global.DefineType("dword", mcf::Object::TypeInfo::MakePrimitive("dword", 4));
				scopeTree.Global.DefineType("qword", mcf::Object::TypeInfo::MakePrimitive("qword", 8));
				mcf::IR::Pointer object = evaluator.Eval(&program, &scopeTree.Global);

				FATAL_ASSERT(object.get() != nullptr, u8"object가 nullptr이면 안됩니다.");
				const std::string actual = object->Inspect();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());

			}
			return true;
		}
	);
	_names.emplace_back(u8"main 평가 테스트");
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
					"main(void) -> void {}",
					"main proc\n"
						"\tpush rbp\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp\n",
				},
				{
					"main(void) -> void { let var1: dword = 15; unused(var1); }",
					"main proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 16\n"
						"\tmov dword ptr [rsp + 0], 15\n" // var1 = 15;
						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp\n",
				},
				{
					"main(void) -> void { let message: byte[] = \"Hello, World!Value = %d\\n\"; unused(byte); }",
					"main proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 16\n"
						
						/* CopyMemory("Hello, World!Value = %d\\n\", message, sizeof(message)); */
						"\tsub rsp, 16\n"
						"\tmov r8, sizeof ?0\n"
						"\tlea rdx, [rsp + 16]\n"
						"\tlea rcx, [?0]\n"
						"\tcall ?CopyMemory\n"
						"\tadd rsp, 16\n"

						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp\n",
				},
				{
					"main(void) -> void { let message: byte[] = \"Hello, World!\\n\"; printf(&message); }",
					"main proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 16\n"
						
						/* CopyMemory(&0, message, sizeof(message)); */
						"\tsub rsp, 16\n"
						"\tmov r8, sizeof ?0\n"
						"\tlea rdx, [rsp + 16]\n"
						"\tlea rcx, [?0]\n"
						"\tcall ?CopyMemory\n"
						"\tadd rsp, 16\n"

						/* printf(&message); */
						"\tsub rsp, 16\n"
						"\tlea rcx, [rsp + 16]\n"
						"\tcall printf\n"
						"\tadd rsp, 16\n"

						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp\n",
				},
				{
					"let intVal: int32 = 10; main(void) -> void { let message: byte[] = \"Hello, World!\\n\"; printf(&message); }",
					"main proc\n"
					"main proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 16\n"
						
						/* CopyMemory(&0, message, sizeof(message)); */
						"\tsub rsp, 16\n"
						"\tmov r8, sizeof ?0\n"
						"\tlea rdx, [rsp + 16]\n"
						"\tlea rcx, [?0]\n"
						"\tcall ?CopyMemory\n"
						"\tadd rsp, 16\n"

						/* printf(&message); */
						"\tsub rsp, 16\n"
						"\tlea rcx, [rsp + 16]\n"
						"\tcall printf\n"
						"\tadd rsp, 16\n"

						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp\n",
				},
			};
			constexpr const size_t testCaseCount = MCF_ARRAY_SIZE(testCases);
			for (size_t i = 0; i < testCaseCount; i++)
			{
				mcf::Parser::Object parser(testCases[i].Input, false);
				mcf::AST::Program program;
				parser.ParseProgram(program);
				FATAL_ASSERT(CheckParserErrors(parser), u8"파싱에 실패 하였습니다.");
				mcf::Evaluator::Object evaluator;
				mcf::Object::ScopeTree scopeTree;
				scopeTree.Global.DefineType("byte", mcf::Object::TypeInfo::MakePrimitive("byte", 1));
				scopeTree.Global.DefineType("word", mcf::Object::TypeInfo::MakePrimitive("word", 2));
				scopeTree.Global.DefineType("dword", mcf::Object::TypeInfo::MakePrimitive("dword", 4));
				scopeTree.Global.DefineType("qword", mcf::Object::TypeInfo::MakePrimitive("qword", 8));
				mcf::IR::Pointer object = evaluator.Eval(&program, &scopeTree.Global);

				FATAL_ASSERT(object.get() != nullptr, u8"object가 nullptr이면 안됩니다.");
				const std::string actual = object->Inspect();
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());

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