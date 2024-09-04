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
				const std::vector<std::string> Literals;
				const std::string Expected;
			} testCases[] =
			{
				{
					"func foo(void) -> void {}",
					{},
					"foo proc\n"
						"\tpush rbp\n"
						"\tpop rbp\n"
						"\tret\n"
					"foo endp\n",
				},
				{
					"func boo(param1: dword) -> void { unused(param1); }",
					{},
					"boo proc\n"
						"\tpush rbp\n"
						"\tmov qword ptr [rsp + 16], rcx\n" // param1 = rcx;
						"\tpop rbp\n"
						"\tret\n"
					"boo endp\n",
				},
				{
					"func boo(void) -> void { let var1: dword = 15; unused(var1); }",
					{},
					"boo proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 16\n"
						"\tmov dword ptr [rsp + 0], 15\n" // var1 = 15;
						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"boo endp\n",
				},
				{
					"func boo(void) -> byte { return 100; }",
					{},
					"boo proc\n"
						"\tpush rbp\n"
						"\tmov al, 100\n"
						"\tpop rbp\n"
						"\tret\n"
					"boo endp\n",
				},
				{
					"func boo(param1: dword) -> dword { return param1; }",
					{},
					"boo proc\n"
						"\tpush rbp\n"
						"\tmov qword ptr [rsp + 16], rcx\n" // param1 = rcx;
						"\tmov eax, dword ptr [rsp + 16]\n"
						"\tpop rbp\n"
						"\tret\n"
					"boo endp\n",
				},
				{
					"func boo(void) -> dword { let var1: dword = 15; return var1; }",
					{},
					"boo proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 16\n"
						"\tmov dword ptr [rsp + 0], 15\n"	// var1 = 15;
						"\tmov eax, dword ptr [rsp + 0]\n"	// return val1;
						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"boo endp\n",
				},
				{
					"func boo(param1: dword) -> dword { let var1: dword = 15; unused(param1); return var1; }",
					{},
					"boo proc\n"
						"\tpush rbp\n"
						"\tmov qword ptr [rsp + 16], rcx\n" // param1 = rcx;
						"\tsub rsp, 16\n"
						"\tmov dword ptr [rsp + 0], 15\n"	// var1 = 15;
						"\tmov eax, dword ptr [rsp + 0]\n"	// return val1;
						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"boo endp\n",
				},
				{
					"func boo(param1: dword) -> dword { let var1: dword = param1; return var1; }",
					{},
					"boo proc\n"
						"\tpush rbp\n"
						"\tmov qword ptr [rsp + 16], rcx\n" // param1 = rcx;
						"\tsub rsp, 16\n"
						"\tmov eax, dword ptr [rsp + 32]\n"	// eax = param1;
						"\tmov dword ptr [rsp + 0], eax\n"	// var1 = eax;
						"\tmov eax, dword ptr [rsp + 0]\n"	// return val1;
						"\tadd rsp, 16\n"
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

				const size_t constantCount = scopeTree.LiteralIndexMap.size();
				FATAL_ASSERT(constantCount == testCases[i].Literals.size(), u8"상수의 갯수가 예상되는 갯수와 다릅니다. 실제값[%zu] 예상값[%zu]", constantCount, testCases[i].Literals.size());
				for (size_t j = 0; j < constantCount; ++j)
				{
					auto literalPairIter = scopeTree.LiteralIndexMap.find(testCases[i].Literals[i]);
					FATAL_ASSERT(literalPairIter == scopeTree.LiteralIndexMap.end(), u8"예상되는 값을 실제 리터럴맵에서 찾을 수 없습니다. 인덱스[%zu] 예상값[%s]", j, testCases[i].Literals[j].c_str());
					FATAL_ASSERT(literalPairIter->second == i, u8"실제값의 인덱스가 예상값의 인덱스와 다릅니다. 실제 인덱스[%zu] 예상 인덱스[%zu]", literalPairIter->second, j);
				}

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
				const std::vector<std::string> Literals;
				const std::string Expected;
			} testCases[] =
			{
				{
					"main(void) -> void {}",
					{},
					"main proc\n"
						"\tpush rbp\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp\n",
				},
				{
					"main(void) -> void { let var1: dword = 15; unused(var1); }",
					{},
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
					{"Hello, World!Value = %d\\n"},
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
					{"Hello, World!Value = %d\\n"},
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
					{"Hello, World!Value = %d\\n"},
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

				const size_t constantCount = scopeTree.LiteralIndexMap.size();
				FATAL_ASSERT(constantCount == testCases[i].Literals.size(), u8"상수의 갯수가 예상되는 갯수와 다릅니다. 실제값[%zu] 예상값[%zu]", constantCount, testCases[i].Literals.size());
				for (size_t j = 0; j < constantCount; ++j)
				{
					auto literalPairIter = scopeTree.LiteralIndexMap.find(testCases[i].Literals[i]);
					FATAL_ASSERT(literalPairIter == scopeTree.LiteralIndexMap.end(), u8"예상되는 값을 실제 리터럴맵에서 찾을 수 없습니다. 인덱스[%zu] 예상값[%s]", j, testCases[i].Literals[j].c_str());
					FATAL_ASSERT(literalPairIter->second == i, u8"실제값의 인덱스가 예상값의 인덱스와 다릅니다. 실제 인덱스[%zu] 예상 인덱스[%zu]", literalPairIter->second, j);
				}

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