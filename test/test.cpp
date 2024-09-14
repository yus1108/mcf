#include <Windows.h>
#include <iostream>

#include "unittest/unittest.h"

#define MAIN_UNIT_TEST(NAME) { UnitTest::##NAME##Test Test##NAME##; if (UnitTest::InternalTest(#NAME, &Test##NAME##) == false) { return 1; } }

const int main(const size_t argc, const char* const argv[])
{
	UnitTest::DetectMemoryLeak();

	// #12 [문자열] C4566 유니코드 컴파일 경고
	// 1. cmd 를 이용하는 경우 시스템 locale 이 .utf8 이 아닌 경우 글자가 깨지는 현상이 있어 콘솔 프로젝트에서 강제로 고정하기로 결정.
	// 2. cmd 를 이용하는 경우 코드 페이지로 인해 글자가 깨지는 현상이 있지만 이 경우에는 코드 페이지가 고정으로 설정되어도 로컬 머신의
    //    환경에 따라 글자가 깨지는 현상이 있어 command line arguments 로 --CodePage=<unsigned_integer> 값을 받아 처리하도록 함
	std::locale::global(std::locale(".UTF8"));
	if ( argc > 1 )
    {
#ifdef _WIN32
		const char cpOption[] = "--CodePage=";
#endif

        std::cout << "options: ";
		for (size_t i = 1; i < argc; ++i)
		{
			std::cout << argv[i] << " ";
#ifdef _WIN32
            constexpr size_t cpOptionLength = sizeof(cpOption) - 1;
			if (strncmp(cpOption, argv[i], cpOptionLength) == 0)
			{
                const size_t cpValueSize = strlen(argv[i]) - cpOptionLength + 1;

                char* cpValue = new char[cpValueSize];
                strncpy_s(cpValue, cpValueSize, argv[i] + cpOptionLength, cpValueSize);
				const UINT codePageID = static_cast<UINT>(atoi(cpValue));
				delete[] cpValue;

				SetConsoleOutputCP(codePageID);
			}
#endif
		}
		std::cout << std::endl;
    }

	// lexer test
	MAIN_UNIT_TEST(Lexer);

	// parser test
	MAIN_UNIT_TEST(Parser);

	// evaluator test
	MAIN_UNIT_TEST(Evaluator);

	// compiler test
	MAIN_UNIT_TEST(Compiler);

	std::cout << "All Tests Passed" << std::endl;
	return 0;
}