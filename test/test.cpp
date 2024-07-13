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

#define unused(variable) variable

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
	"invalid",

	"variable_declaration",
};
constexpr const size_t STATEMENT_TYPES_SIZE = array_size(STATEMENT_TYPES);
static_assert(static_cast<size_t>(mcf::ast::statement_type::count) == STATEMENT_TYPES_SIZE, "statement_type count not matching");

constexpr const std::string_view EXPRESSION_TYPES[] =
{
	"invalid",

	"literal",
	"identifier",
	"data_type",
};
constexpr const size_t EXPRESSION_TYPES_SIZE = array_size(EXPRESSION_TYPES);
static_assert(static_cast<size_t>(mcf::ast::expression_type::count) == EXPRESSION_TYPES_SIZE, "expression_type count not matching");

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
			fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable_declaration, 
				u8"statement가 variable_declaration이 아닙니다. 결과값=%s", 
				STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())].data());

			const mcf::ast::variable_declaration_statement* variableDeclaration = static_cast<const mcf::ast::variable_declaration_statement*>(statement);
			fatal_assert(variableDeclaration->get_type() == expectedDataType, u8"변수 선언 타입이 '%s'가 아닙니다. 실제값=%s", TOKEN_TYPES[enum_index(expectedDataType)].data(), TOKEN_TYPES[enum_index(variableDeclaration->get_type())].data());
			fatal_assert(variableDeclaration->get_name() == expectedName, u8"변수 선언 이름이 '%s'가 아닙니다. 실제값=%s", expectedName.c_str(), variableDeclaration->get_name().c_str());
			// TODO: #11 initialization 도 구현 필요

			return true;
		}
	}

	constexpr const size_t PARSER_TEST_FUNCTIONS_COUNT_BEGIN = __COUNTER__;
	bool test_variable_declaration_statements(const size_t function_number = __COUNTER__)
	{
		unused(function_number);

		// TODO: #11 initialization 도 구현 필요
		const std::string input = "int32 x; int32 y = 10; int32 foobar = 838383;";
		
		mcf::parser parser = mcf::parser(input);
		mcf::ast::program program;
		parser.parse_program(program);
		const size_t statementCount = program.get_statement_count();
		
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
			const mcf::ast::statement* statement = program.get_statement_at(i);
			if (internal::test_variable_declaration_statement(statement, testCases[i].ExpectedDataType, testCases[i].ExpectedIdentifier) == false)
			{
				return false;
			}
		}
		return true;
	}

	bool test_convert_to_string(const size_t function_number = __COUNTER__)
	{
		unused(function_number);

		mcf::ast::data_type_expression dataType({ mcf::token_type::integer_32bit, "int32" });
		mcf::ast::identifier_expression name({ mcf::token_type::identifier, "myVar" });
		mcf::ast::identifier_expression* rightExpression = new(std::nothrow) mcf::ast::identifier_expression({ mcf::token_type::identifier, "anotherVar" });
		mcf::ast::variable_declaration_statement *variableDeclarationStatement = new(std::nothrow) mcf::ast::variable_declaration_statement(dataType, name, rightExpression);
		std::vector<const mcf::ast::statement*> statements =
		{
			variableDeclarationStatement,
		};

		mcf::ast::program program(statements);
		fatal_assert(program.convert_to_string() == "int32 myVar = anotherVar;\n", u8"program의 string 변환이 틀렸습니다. 실제값=`%s`", program.convert_to_string().c_str());
		
		return true;
	}

	bool test_identifier_expression(const size_t function_number = __COUNTER__)
	{
		unused(function_number);

		const std::string input = "int32 foo = bar;";
		mcf::parser parser(input);
		mcf::ast::program program;
		parser.parse_program(program);

		fatal_assert(program.get_statement_count() == 1, u8"program._statements 안에 3개의 명령문을 가지고 있어야 합니다. 결과값=%zu", program.get_statement_count());
		const mcf::ast::statement* statement = program.get_statement_at(0);

		fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable_declaration, u8"statement의 statement type이 variable_declaration가 아닙니다. statement=%s", 
			STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())].data());
		const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_declaration_statement*>(statement)->get_init_expression();

		fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::identifier, u8"expression의 expression type이 identifier가 아닙니다. identifier=%s",
			EXPRESSION_TYPES[mcf::enum_index(initExpression->get_expression_type())].data());

		const mcf::ast::identifier_expression* identifier = static_cast<const mcf::ast::identifier_expression*>(initExpression);
		fatal_assert(identifier->get_token().Literal == "bar", u8"identifier의 literal값이 `bar`가 아닙니다. identifier.literal=`%s`", identifier->get_token().Literal.c_str());

		return true;
	}

	bool test_literal_expression(const size_t function_number = __COUNTER__)
	{
		unused(function_number);

		const std::string input = "int32 foo = 5;";
		mcf::parser parser(input);
		mcf::ast::program program;
		parser.parse_program(program);

		fatal_assert(program.get_statement_count() == 1, u8"program._statements 안에 3개의 명령문을 가지고 있어야 합니다. 결과값=%zu", program.get_statement_count());
		const mcf::ast::statement* statement = program.get_statement_at(0);

		fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable_declaration, u8"statement의 statement type이 variable_declaration가 아닙니다. statement=%s",
			STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())].data());
		const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_declaration_statement*>(statement)->get_init_expression();

		fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::literal, u8"expression의 expression type이 identifier가 아닙니다. identifier=%s",
			EXPRESSION_TYPES[mcf::enum_index(initExpression->get_expression_type())].data());

		const mcf::ast::literal_expession* literal = static_cast<const mcf::ast::literal_expession*>(initExpression);
		fatal_assert(literal->get_token().Literal == "5", u8"identifier의 literal값이 `5`가 아닙니다. identifier.literal=`%s`", literal->get_token().Literal.c_str());

		return true;
	}
	constexpr const size_t PARSER_TEST_FUNCTIONS_COUNT = PARSER_TEST_FUNCTIONS_COUNT_BEGIN - __COUNTER__ - 1;
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

	// lexer test
	{
		if (lexer_test::test_next_token() == false)
		{
			std::cout << "Test `test_next_token()` Failed" << std::endl;
			return 1;
		}
	}

	// parser test
	{
		constexpr const size_t PARSER_TEST_FUNCTIONS_CALLED_COUNT_BEGIN = __COUNTER__;
		if (parser_test::test_variable_declaration_statements(__COUNTER__) == false)
		{
			std::cout << "Test `test_variable_declaration_statements()` Failed" << std::endl;
			return 1;
		}

		if (parser_test::test_convert_to_string(__COUNTER__) == false)
		{
			std::cout << "Test `test_convert_to_string()` Failed" << std::endl;
			return 1;
		}

		if (parser_test::test_identifier_expression(__COUNTER__) == false)
		{
			std::cout << "Test `test_convert_to_string()` Failed" << std::endl;
			return 1;
		}
		
		if (parser_test::test_literal_expression(__COUNTER__) == false)
		{
			std::cout << "Test `test_convert_to_string()` Failed" << std::endl;
			return 1;
		}
		constexpr const size_t PARSER_TEST_FUNCTIONS_CALLED_COUNT = PARSER_TEST_FUNCTIONS_CALLED_COUNT_BEGIN - __COUNTER__ - 1;
		static_assert(parser_test::PARSER_TEST_FUNCTIONS_COUNT == PARSER_TEST_FUNCTIONS_CALLED_COUNT, "must add newly added parser test function");
	}
	

    std::cout << "All Test Passed" << std::endl;
	return 0;
}