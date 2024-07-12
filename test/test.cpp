#include <iostream>
#include <vector>
#include <memory>
#include <string>

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

#if defined(_DEBUG)
#define fatal_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); return false; } ((void)0)
#else
#define fatal_assert(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); return false; } ((void)0)
#endif

#define error_message_begin(ERROR_COUNT) printf(u8"[Error]: %s(Line: %d) %zu개의 에러 메시지가 있습니다.\n", ##__FILE__, ##__LINE__, ERROR_COUNT)
#define error_message(FORMAT, ...) printf(FORMAT, __VA_ARGS__); printf("\n"); ((void)0)
#if defined(_DEBUG)
#define error_message_end __debugbreak(); abort();
#else
#define error_message_end abort();
#endif

// ENUM
#define enum_count(enum) static_cast<size_t>(##enum##::count)

template<typename T>
constexpr const size_t enum_index(const T value)
{
	static_assert(std::is_enum_v<T> == true, u8"해당 함수는 enum value만 사용 가능합니다");
	return static_cast<size_t>(value);
}

template<typename T>
const T enum_at(const size_t index)
{
	static_assert(std::is_enum_v<T> == true, u8"해당 함수는 enum value만 사용 가능합니다.");
	debug_assert(index < enum_count(T), u8"index가 해당 enum의 크기보다 큽니다. index=%zu", index);
	return static_cast<T>(index);
}

// ARRAY
template<typename T>
constexpr const size_t array_type_size(T array[])
{
	array;
	return sizeof(T);
}

#define array_size(array) sizeof(array) / array_type_size(array);

#include <lexer/includes/lexer.h>
constexpr const std::string_view TOKEN_TYPES[] =
{
	"invalid",
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
constexpr const size_t TOKEN_TYPES_SIZE = array_size(TOKEN_TYPES);
static_assert(static_cast<size_t>(mcf::token_type::count) == TOKEN_TYPES_SIZE, u8"토큰의 갯수가 일치 하지 않습니다. 수정이 필요합니다!");

#include <parser/includes/ast.h>
#include <parser/includes/parser.h>
constexpr const std::string_view STATEMENT_TYPES[] =
{
	"variable_declaration",
};
constexpr const size_t STATEMENT_TYPES_SIZE = array_size(STATEMENT_TYPES);
static_assert(static_cast<size_t>(mcf::ast::statement_type::count) == STATEMENT_TYPES_SIZE, u8"statement_type의 갯수가 일치 하지 않습니다. 수정이 필요합니다!");

namespace lexer_test
{
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
		} testCases[] =
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
					{mcf::token_type::keyword_int32, "int32"},
					{mcf::token_type::identifier, "foo"},
					{mcf::token_type::assign, "="},
					{mcf::token_type::integer_32bit, "5"},
					{mcf::token_type::semicolon, ";"},
				},
			},
			{
				"int32 foo = 5 + 5 - 8 * 4 / 2;",
				{
					{mcf::token_type::keyword_int32, "int32"},
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
					{mcf::token_type::keyword_int32, "int32"},
					{mcf::token_type::identifier, "foo"},
					{mcf::token_type::assign, "="},
					{mcf::token_type::minus, "-"},
					{mcf::token_type::integer_32bit, "1"},
					{mcf::token_type::semicolon, ";"},
				},
			},
		};
		constexpr const size_t testCaseCount = array_size(testCases);

		for (size_t i = 0; i < testCaseCount; i++)
		{
			const size_t vectorSize = testCases[i].ExpectedResultVector.size();
			mcf::lexer lexer(testCases[i].Input);
			for (size_t j = 0; j < vectorSize; j++)
			{
				const mcf::token token = lexer.read_next_token();
				const mcf::token_type expectedTokenType = testCases[i].ExpectedResultVector[j].Type;

				fatal_assert(token.Type == expectedTokenType, u8"tests[%zu-%zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
					i, j, TOKEN_TYPES[enum_index(expectedTokenType)].data(), TOKEN_TYPES[enum_index(token.Type)].data());

				fatal_assert(token.Literal == testCases[i].ExpectedResultVector[j].Literal, u8"tests[%zu-%zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
					i, j, testCases[i].ExpectedResultVector[j].Literal.data(), token.Literal.data());
			}
		}

		return true;
	}
}

namespace parser_test
{
	namespace internal
	{
		void check_parser_errors(mcf::parser& parser)
		{
			const size_t errorCount = parser.get_error_count();
			if (errorCount == 0)
			{
				return;
			}

			error_message_begin(errorCount);
			mcf::parser::error curr = parser.get_last_error();
			while (curr.ID != mcf::parser::error::id::no_error)
			{
				error_message(u8"p%zu, 파싱 에러: %s", enum_index(curr.ID), curr.Message.c_str());
				curr = parser.get_last_error();
			}
			error_message_end;
		}

		bool test_variable_declaration_statement(const mcf::ast::statement* statement, const mcf::token_type expectedDataType,  const std::string& expectedName)
		{
			fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable_declaration, u8"statement가 variable_declaration이 아닙니다. 결과값=%s", 
				STATEMENT_TYPES[enum_index(statement->get_statement_type())].data());

			const mcf::ast::variable_declaration_statement* variableDeclaration = static_cast<const mcf::ast::variable_declaration_statement*>(statement);
			fatal_assert(variableDeclaration->get_type() == expectedDataType, u8"변수 선언 타입이 '%s'가 아닙니다. 실제값=%s", TOKEN_TYPES[enum_index(expectedDataType)].data(), TOKEN_TYPES[enum_index(variableDeclaration->get_type())].data());
			fatal_assert(variableDeclaration->get_name() == expectedName, u8"변수 선언 이름이 '%s'가 아닙니다. 실제값=%s", expectedName.c_str(), variableDeclaration->get_name().c_str());
			// TODO: #11 initialization 도 구현 필요

			return true;
		}
	}

	bool test_variable_declaration_statements( void )
	{
		// TODO: #11 initialization 도 구현 필요
		const std::string input = "int32 x; int32 y = 10; int32 foobar = 838383;";
		
		mcf::parser parser = mcf::parser(input);
		std::vector<const mcf::ast::statement*> unsafeProgram;
		parser.parse_program(unsafeProgram);
		const size_t statementCount = unsafeProgram.size();
		std::unique_ptr<std::unique_ptr<const mcf::ast::statement>[]> program = std::make_unique<std::unique_ptr<const mcf::ast::statement>[]>(statementCount);
		for (size_t i = 0; i < statementCount; i++)
		{
			program[i] = std::unique_ptr<const mcf::ast::statement>(unsafeProgram[i]);
		}
		
		internal::check_parser_errors(parser);
		fatal_assert(statementCount == 3, u8"program._statements 안에 3개의 명령문을 가지고 있어야 합니다. 결과값=%zu", statementCount);

		const struct test_case
		{
			const mcf::token_type	ExpectedDataType;
			const std::string		ExpectedIdentifier;
		} testCases[] =
		{
			{mcf::token_type::keyword_int32, "x"},
			{mcf::token_type::keyword_int32, "y"},
			{mcf::token_type::keyword_int32, "foobar"},
		};
		constexpr const size_t testCaseCount = array_size(testCases);

		for (size_t i = 0; i < testCaseCount; i++)
		{
			const mcf::ast::statement* statement = program[i].get();
			if (internal::test_variable_declaration_statement(statement, testCases[i].ExpectedDataType, testCases[i].ExpectedIdentifier) == false)
			{
				return false;
			}
		}
		return true;
	}
};

int main(const size_t argc, const char* const argv[])
{
	detect_memory_leak();

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
		for (int i = 1; i < argc; ++i)
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


    if (lexer_test::test_next_token() == false)
    {
        std::cout << "Test `test_next_token()` Failed" << std::endl;
        return 1;
    }

	if ( parser_test::test_variable_declaration_statements() == false )
	{
		std::cout << "Test `test_next_token()` Failed" << std::endl;
		return 1;
	}
    std::cout << "All Test Passed" << std::endl;
	return 0;
}