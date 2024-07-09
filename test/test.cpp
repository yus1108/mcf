#include <iostream>
#include <vector>

#ifdef _DEBUG

#ifdef _WIN32
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

constexpr const std::wstring_view gTokenTypeWStringArray[] =
{
    L"illegal",
    L"eof",

    // 식별자 + 리터럴
    L"identifier",
    L"integer",

    // 연산자
    L"assign",
    L"plus",
    L"minus",
    L"asterisk",
    L"slash",

    // 구분자
    L"semicolon",

    // 예약어
    L"keyword",
};
static_assert(static_cast<size_t>(mcf::token_type::count) == (sizeof(gTokenTypeWStringArray) / sizeof(std::wstring_view)), L"토큰의 갯수가 일치 하지 않습니다. 수정이 필요합니다!");

#if defined(_DEBUG)
#define wfatal_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { wprintf(FORMAT, __VA_ARGS__); __debugbreak(); return false; } ((void)0)
#else
#define wfatal_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { wprintf(FORMAT, __VA_ARGS__); return false; } ((void)0)
#endif

std::wstring convert_to_wstring(const std::string& value)
{
    return std::wstring(value.begin(), value.end());
}
std::wstring convert_to_wstring(const std::string_view& value)
{
	return std::wstring(value.begin(), value.end());
}

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
			const mcf::token_type lTokenType = lToken.Type;
			const mcf::token_type lExpectedTokenType = lTestCase[i].ExpectedResultVector[j].Type;

            wfatal_assert(lTokenType == lExpectedTokenType, L"tests[%llu-%llu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
                i, j, gTokenTypeWStringArray[static_cast<size_t>(lExpectedTokenType)].data(), gTokenTypeWStringArray[static_cast<size_t>(lTokenType)].data());

            wfatal_assert(lToken.Literal == lTestCase[i].ExpectedResultVector[j].Literal, L"tests[%llu-%llu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
                i, j, convert_to_wstring(lTestCase[i].ExpectedResultVector[j].Literal).c_str(), convert_to_wstring(lToken.Literal).c_str());
        }
    }

    return true;
}

int main()
{
    detect_memory_leak();

    if (test_next_token() == false)
    {
        std::cout << "Test `test_next_token()` Failed" << std::endl;
        return 1;
    }
    std::cout << "All Test Passed" << std::endl;
    return 0;
}