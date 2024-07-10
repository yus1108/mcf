#include <iostream>
#include <vector>

#ifdef _DEBUG

#ifdef _WIN32
#include <Windows.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

inline void detect_memory_leak(long line = -1)
{
    //Also need this for memory leak code stuff
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetBreakAlloc(line); //Important!
}

#else // other platforms
inline void detect_memory_leak(long line = -1)
{
    line;
#error "must be implemented"
}
#endif // platform

#else // other configurations

inline void detect_memory_leak(long line = -1) { line; }

#endif // configurations

#include <lexer.h>

constexpr const std::string_view gTokenTypeWStringArray[] =
{
    "illegau8",
    "eof",

    // 식별자 + 리터럴
    "identifier",
    "integer",

    // 연산자
    "assign",
    "plus",
    "minus",
    "asterisk",
    "slash",

    // 구분자
    "semicolon",

    // 예약어
    "keyword",
};
static_assert(static_cast<size_t>(mcf::token_type::count) == (sizeof(gTokenTypeWStringArray) / sizeof(std::string_view)), u8"토큰의 갯수가 일치 하지 않습니다. 수정이 필요합니다!");

#if defined(_DEBUG)
#define fatal_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf(FORMAT, __VA_ARGS__); __debugbreak(); return false; } ((void)0)
#else
#define fatal_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf(FORMAT, __VA_ARGS__); return false; } ((void)0)
#endif

bool test_next_token()
{
    struct expected_result final
    {
        const mcf::token_type   Type;
        const std::string_view  Literal;
    };

    const struct test_case
    {
        const std::string                   Input;
        const std::vector<expected_result>  ExpectedResultVector;
    } lTestCase[] =
    {
        {
            // TODO: constexpr std::string_view lInput = "=+-*/(){},;";
            "=+-*/;",
            {
                {mcf::token_type::assign, "="},
                {mcf::token_type::plus, "+"},
                {mcf::token_type::minus, "-"},
                {mcf::token_type::asterisk, "*"},
                {mcf::token_type::slash, "/"},
                // TODO: {mcf::token_type::lparen, "("},
                // TODO: {mcf::token_type::rparen, ")"},
                // TODO: {mcf::token_type::lbrace, "{"},
                // TODO: {mcf::token_type::rbrace, "}"},
                // TODO: {mcf::token_type::comma, ","},
                {mcf::token_type::semicolon, ";"},
            },
        },
        {
            "int32 foo = 5;",
            {
                {mcf::token_type::keyword, "int32"},
                {mcf::token_type::identifier, "foo"},
                {mcf::token_type::assign, "="},
                {mcf::token_type::integer_32bit, "5"},
                {mcf::token_type::semicolon, ";"},
            },
        },
        {
            "int32 foo = 5 + 5 - 8 * 4 / 2;",
            {
                {mcf::token_type::keyword, "int32"},
                {mcf::token_type::identifier, "foo"},
                {mcf::token_type::assign, "="},
                {mcf::token_type::integer_32bit, "5"},
                {mcf::token_type::plus, "+"},
                {mcf::token_type::integer_32bit, "5"},
                {mcf::token_type::minus, "-"},
                {mcf::token_type::integer_32bit, "8"},
                {mcf::token_type::asterisk, "*"},
                {mcf::token_type::integer_32bit, "4"},
                {mcf::token_type::slash, "/"},
                {mcf::token_type::integer_32bit, "2"},
                {mcf::token_type::semicolon, ";"},
            },
        },
        {
            "int32 foo = -1;",
            {
                {mcf::token_type::keyword, "int32"},
                {mcf::token_type::identifier, "foo"},
                {mcf::token_type::assign, "="},
                {mcf::token_type::minus, "-"},
                {mcf::token_type::integer_32bit, "1"},
                {mcf::token_type::semicolon, ";"},
            },
        },
    };
    constexpr const size_t lTestCaseCount = sizeof(lTestCase) / sizeof(test_case);

	for (size_t i = 0; i < lTestCaseCount; i++)
    {
        const size_t lVectorSize = lTestCase[i].ExpectedResultVector.size();
        mcf::lexer lLexer(lTestCase[i].Input);
        for (size_t j = 0; j < lVectorSize; j++)
        {
            const mcf::token lToken = lLexer.read_next_token();

            fatal_assert( lToken.Type == lTestCase[i].ExpectedResultVector[j].Type, u8"tests[%zu-%zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
                i, j, gTokenTypeWStringArray[static_cast<size_t>(lTestCase[i].ExpectedResultVector[j].Type)].data(), gTokenTypeWStringArray[static_cast<size_t>(lToken.Type)].data());

            fatal_assert(lToken.Literal == lTestCase[i].ExpectedResultVector[j].Literal, u8"tests[%zu-%zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
                i, j, lTestCase[i].ExpectedResultVector[j].Literal.data(), lToken.Literal.data());
        }
    }

    return true;
}

int main(const size_t argc, const char* const argv[])
{
	detect_memory_leak();

	// #12 [문자열] C4566 유니코드 컴파일 경고
	// 1. cmd 를 이용하는 경우 시스템 locale 이 .utf8 이 아닌 경우 글자가 깨지는 현상이 있어 콘솔 프로젝트에서 강제로 고정하기로 결정.
	// 2. cmd 를 이용하는 경우 코드 페이지로 인해 글자가 깨지는 현상이 있지만 이 경우에는 코드 페이지가 고정으로 설정되어도 로컬 머신의
    //    환경에 따라 글자가 깨지는 현상이 있어 command line arguments 로 --CodePage=<unsigned_integer> 값을 받아 처리하도록 함
	std::locale::global( std::locale( ".UTF8" ) );
	if ( argc > 1 )
    {
#ifdef _WIN32
		const char lCP[] = "--CodePage=";
#endif

        std::cout << "options: ";
		for ( int i = 1; i < argc; ++i )
		{
			std::cout << argv[i] << " ";
#ifdef _WIN32
            constexpr size_t lCPLenth = sizeof( lCP ) - 1;
			if ( strncmp( lCP, argv[i], lCPLenth ) == 0 )
			{
                const size_t lCPValueSize = strlen( argv[i] ) - lCPLenth + 1;

                char* lCPValue = new char[lCPValueSize];
                strncpy_s( lCPValue, lCPValueSize, argv[i] + lCPLenth, lCPValueSize );
				const UINT lCodePageID = static_cast<UINT>(atoi( lCPValue ));
				delete[] lCPValue;

				SetConsoleOutputCP( lCodePageID );
			}
#endif
		}
		std::cout << std::endl;
    }


    if (test_next_token() == false)
    {
        std::cout << "Test `test_next_token()` Failed" << std::endl;
        return 1;
    }
    std::cout << "All Test Passed" << std::endl;
	return 0;
}