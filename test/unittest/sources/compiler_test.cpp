#include <iostream>
#include "../unittest.h"

UnitTest::CompilerTest::CompilerTest(void) noexcept
{
	_names.emplace_back(u8"no section 명령어 컴파일 테스트");
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
					"int32 typedef dword",
				},
				{
					"typedef uint32: unsigned dword;",
					"uint32 typedef dword",
				},
				{
					"typedef address: unsigned qword;",
					"address typedef qword",
				},
				{
					"extern func printf(format: unsigned qword, ...args) -> dword;",
					"printf PROTO, format:qword, args:VARARG",
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
				mcf::IR::Program::Pointer irProgram = evaluator.EvalProgram(&program, &scopeTree.Global);
				FATAL_ASSERT(irProgram.get() != nullptr, u8"irProgram이 nullptr이면 안됩니다.");
				FATAL_ASSERT(irProgram->GetType() == mcf::IR::Type::PROGRAM, u8"irProgram이 IR::Type::PROGRAM이 아닙니다.");

				mcf::ASM::MASM64::Compiler::Object compiler;
				mcf::ASM::PointerVector generatedCodes = compiler.GenerateCodes(irProgram.get(), &scopeTree);
				std::string actual;
				const size_t codeCount = generatedCodes.size();
				for (size_t j = 0; j < codeCount; j++)
				{
					actual += (j == 0 ? "" : "\n") + generatedCodes[j]->ConvertToString();
				}
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\n\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);

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
					"typedef bool: byte;"
					"let false: bool = 0;"
					"let true: bool = 1;",
					"bool typedef byte\n"
					".data\n"
					"false bool 0,\n"
					"true bool 1,",
				},
				{
					"let arr2: byte[5] = { 0 };",
					".data\n"
					"arr2 byte 0,",
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
				mcf::IR::Program::Pointer irProgram = evaluator.EvalProgram(&program, &scopeTree.Global);
				FATAL_ASSERT(irProgram.get() != nullptr, u8"irProgram이 nullptr이면 안됩니다.");
				FATAL_ASSERT(irProgram->GetType() == mcf::IR::Type::PROGRAM, u8"irProgram이 IR::Type::PROGRAM이 아닙니다.");

				mcf::ASM::MASM64::Compiler::Object compiler;
				mcf::ASM::PointerVector generatedCodes = compiler.GenerateCodes(irProgram.get(), &scopeTree);
				std::string actual;
				const size_t codeCount = generatedCodes.size();
				for (size_t j = 0; j < codeCount; j++)
				{
					actual += (j == 0 ? "" : "\n") + generatedCodes[j]->ConvertToString();
				}
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\n\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
			}
			return true;
		}
	);

	_names.emplace_back( u8".code 컴파일 테스트" );
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
					".code\n"
					"boo proc\n"
						"\tpush rbp\n"
						"\txor al, al\n"
						"\tpop rbp\n"
						"\tret\n"
					"boo endp",
				},
				{
					"main(void) -> void {}",
					".code\n"
					"main proc\n"
						"\tpush rbp\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp",
				},
				{
					"func boo(void) -> byte { return 0; }"
					"main(void) -> void {}",
					".code\n"
					"boo proc\n"
						"\tpush rbp\n"
						"\txor al, al\n"
						"\tpop rbp\n"
						"\tret\n"
					"boo endp\n"
					"main proc\n"
						"\tpush rbp\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp",
				},
				{
					"func boo(param1: dword) -> dword { let var1: dword = param1; return var1; }",
					".code\n"
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
					"boo endp",
				},
				{
					"main(void) -> void { let var1: dword = 15; unused(var1); }",
					".code\n"
					"main proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 16\n"
						"\tmov dword ptr [rsp + 0], 15\n" // var1 = 15;
						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp",
				},
				{
					"main(void) -> void { let message: byte[] = \"Hello, World!Value = %d\\n\"; unused(message); }",
					".data\n"
					"?0 byte 72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33, 86, 97, 108, 117, 101, 32, 61, 32, 37, 100, 10, 0,\n"
					".code\n"
					"main proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 32\n"

						/* CopyMemory("Hello, World!Value = %d\\n\", message, sizeof(message)); */
						"\tsub rsp, 32\n"
						"\tmov r8, sizeof ?0\n"
						"\tlea rdx, [rsp + 32]\n"
						"\tlea rcx, [?0]\n"
						"\tcall ?CopyMemory\n"
						"\tadd rsp, 32\n"

						"\tadd rsp, 32\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp",
				},
				{
					"extern func printf(format: unsigned qword, ...args) -> dword;"
					"let intVal: dword = 10;"
					"main(void) -> void { let message: byte[] = \"Hello, World!\\n\"; printf(message as unsigned qword); }",
					".data\n"
					"?0 byte 72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33, 10, 0,\n"
					"printf PROTO, format:qword, args:VARARG\n"
					"intVal dword 10,\n"
					".code\n"
					"main proc\n"
						"\tpush rbp\n"
						"\tsub rsp, 16\n"
						
						/* CopyMemory(&0, message, sizeof(message)); */
						"\tsub rsp, 32\n"
						"\tmov r8, sizeof ?0\n"
						"\tlea rdx, [rsp + 32]\n"
						"\tlea rcx, [?0]\n"
						"\tcall ?CopyMemory\n"
						"\tadd rsp, 32\n"

						/* printf(message as unsigned qword); */
						"\tsub rsp, 32\n"
						"\tlea rcx, [rsp + 32]\n"
						"\tcall printf\n"
						"\tadd rsp, 32\n"

						"\tadd rsp, 16\n"
						"\tpop rbp\n"
						"\tret\n"
					"main endp",
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
				mcf::IR::Program::Pointer irProgram = evaluator.EvalProgram(&program, &scopeTree.Global);
				FATAL_ASSERT(irProgram.get() != nullptr, u8"irProgram이 nullptr이면 안됩니다.");
				FATAL_ASSERT(irProgram->GetType() == mcf::IR::Type::PROGRAM, u8"irProgram이 IR::Type::PROGRAM이 아닙니다.");

				mcf::ASM::MASM64::Compiler::Object compiler;
				mcf::ASM::PointerVector generatedCodes = compiler.GenerateCodes(irProgram.get(), &scopeTree);
				std::string actual;
				const size_t codeCount = generatedCodes.size();
				for (size_t j = 0; j < codeCount; j++)
				{
					actual += (j == 0 ? "" : "\n") + generatedCodes[j]->ConvertToString();
				}
				FATAL_ASSERT(actual == testCases[i].Expected, "\ninput(index: %zu):\n%s\nexpected:\n%s\n\nactual:\n%s", i, testCases[i].Input.c_str(), testCases[i].Expected.c_str(), actual.c_str());
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
