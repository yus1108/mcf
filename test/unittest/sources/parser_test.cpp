#include <iostream>
#include <fstream>
#include "test/unittest/unittest.h"

UnitTest::Parser::Parser(void) noexcept
{
	_names.emplace_back("test_variable_declaration_statements");
	_tests.emplace_back([&]() {
		// TODO: #11 initialization 도 구현 필요
		const std::string input = "int32 x; int32 y = 10; int32 foobar = 838383;";

		mcf::parser parser = mcf::parser(input, false);
		mcf::ast::program program;
		parser.parse_program(program);
		const size_t statementCount = program.get_statement_count();

		if (check_parser_errors(parser) == false)
		{
			return false;
		}
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
			if (test_variable_declaration_statement(statement, testCases[i].ExpectedDataType, testCases[i].ExpectedIdentifier) == false)
			{
				return false;
			}
		}
		return true;
		});

	_names.emplace_back("test_convert_to_string");
	_tests.emplace_back([&]() {
		constexpr size_t CAPACITY_START = __COUNTER__;
		__COUNTER__;
		mcf::ast::data_type_expression dataType(false, { mcf::token_type::integer, "int32" });
		mcf::ast::identifier_expression name({ mcf::token_type::identifier, "myVar" });
		mcf::ast::identifier_expression* rightExpression = new(std::nothrow) mcf::ast::identifier_expression({ mcf::token_type::identifier, "anotherVar" });
		mcf::ast::variable_statement* variableDeclarationStatement = new(std::nothrow) mcf::ast::variable_statement(dataType, name, rightExpression);
		constexpr size_t CAPACITY = __COUNTER__ - CAPACITY_START - 1;

		mcf::ast::statement_array statementArray = std::make_unique<mcf::ast::unique_statement[]>(CAPACITY);
		size_t count = 0;
		{
			statementArray[count++] = mcf::ast::unique_statement(variableDeclarationStatement);
		}

		mcf::ast::program program(statementArray.release(), CAPACITY);
		fatal_assert(program.convert_to_string() == "int32 myVar = anotherVar;", u8"program의 string 변환이 틀렸습니다. 실제값=`%s`", program.convert_to_string().c_str());

		return true;
		});

	_names.emplace_back("test_identifier_expression");
	_tests.emplace_back([&]() {
		const std::string input = "int32 foo = bar;";
		mcf::parser parser(input, false);
		mcf::ast::program program;
		parser.parse_program(program);
		if (check_parser_errors(parser) == false)
		{
			return false;
		}

		fatal_assert(program.get_statement_count() == 1, u8"program._statements 안에 1개의 명령문을 가지고 있어야 합니다. 결과값=%zu", program.get_statement_count());
		const mcf::ast::statement* statement = program.get_statement_at(0);

		fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable, u8"statement의 statement type이 variable가 아닙니다. statement=%s",
			STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);
		const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_statement*>(statement)->get_init_expression();

		fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::identifier, u8"init expression의 expression type이 identifier가 아닙니다. expression_type=%s",
			EXPRESSION_TYPES[mcf::enum_index(initExpression->get_expression_type())]);

		const mcf::ast::identifier_expression* identifier = static_cast<const mcf::ast::identifier_expression*>(initExpression);
		fatal_assert(identifier->get_token().Literal == "bar", u8"identifier의 literal값이 `bar`가 아닙니다. identifier.literal=`%s`", identifier->get_token().Literal.c_str());

		return true;
		});

	_names.emplace_back("test_literal_expression");
	_tests.emplace_back([&]() {
		const std::string input = "int32 foo = 5;";
		mcf::parser parser(input, false);
		mcf::ast::program program;
		parser.parse_program(program);
		if (check_parser_errors(parser) == false)
		{
			return false;
		}

		fatal_assert(program.get_statement_count() == 1, u8"program._statements 안에 1개의 명령문을 가지고 있어야 합니다. 결과값=%zu", program.get_statement_count());
		const mcf::ast::statement* statement = program.get_statement_at(0);

		fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable, u8"statement의 statement type이 variable가 아닙니다. statement=%s",
			STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);
		const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_statement*>(statement)->get_init_expression();

		fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::literal, u8"init expression의 expression type이 literal가 아닙니다. expression_type=%s",
			EXPRESSION_TYPES[mcf::enum_index(initExpression->get_expression_type())]);

		const mcf::ast::literal_expession* literal = static_cast<const mcf::ast::literal_expession*>(initExpression);
		fatal_assert(literal->get_token().Literal == "5", u8"literal의 literal값이 `5`가 아닙니다. identifier.literal=`%s`", literal->get_token().Literal.c_str());

		return true;
		});

	_names.emplace_back("test_prefix_expressions");
	_tests.emplace_back([&]() {
		using namespace std;
		using namespace mcf;
		using namespace mcf::ast;

		const struct test_case
		{
			const string					Input;
			const token						ExpectedPrefixToken;
			unique_ptr<const expression>	ExpectedTargetExpression;
		} testCases[] =
		{
			{"int32 foo = -5;", { token_type::minus, "-" }, make_unique<const literal_expession>(token{ token_type::integer, "5" })},
			// TODO: {"int32 foo = !5;", { token_type::not, "!" }, make_unique<const literal_expession>(token{ token_type::integer, "5" })},
			{"int32 foo = -15;", { token_type::minus, "-" }, make_unique<const literal_expession>(token{ token_type::integer, "15" })},
			{"int32 foo = +5;", { token_type::plus, "+" }, make_unique<const literal_expession>(token{ token_type::integer, "5" })},
			{"int32 foo = +15;", { token_type::plus, "+" }, make_unique<const literal_expession>(token{ token_type::integer, "15" })},
		};
		constexpr const size_t testCaseCount = array_size(testCases);

		for (size_t i = 0; i < testCaseCount; i++)
		{
			mcf::parser parser(testCases[i].Input, false);
			mcf::ast::program program;
			parser.parse_program(program);
			if (check_parser_errors(parser) == false)
			{
				return false;
			}

			fatal_assert(program.get_statement_count() == 1, u8"program._statements 안에 1개의 명령문을 가지고 있어야 합니다. 결과값=%zu", program.get_statement_count());
			const mcf::ast::statement* statement = program.get_statement_at(0);

			fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable, u8"statement의 statement type이 variable가 아닙니다. statement=%s",
				STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);
			const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_statement*>(statement)->get_init_expression();

			fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::prefix, u8"init expression의 expression type이 prefix가 아닙니다. expression_type=%s",
				EXPRESSION_TYPES[mcf::enum_index(initExpression->get_expression_type())]);

			const mcf::ast::prefix_expression* prefixExpression = static_cast<const mcf::ast::prefix_expression*>(initExpression);
			fatal_assert(prefixExpression->get_prefix_operator_token() == testCases[i].ExpectedPrefixToken, u8"prefix operator token이 %s와 다릅니다. token=%s",
				convert_to_string(testCases[i].ExpectedPrefixToken).c_str(), convert_to_string(prefixExpression->get_prefix_operator_token()).c_str());

			const mcf::ast::expression* targetExpression = prefixExpression->get_target_expression();

			constexpr const size_t TARGET_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
			switch (targetExpression->get_expression_type())
			{
			case mcf::ast::expression_type::literal: __COUNTER__;
				fatal_assert(testCases[i].ExpectedTargetExpression->get_expression_type() == mcf::ast::expression_type::literal,
					u8"targetExpression의 expression type은 literal여야 합니다. expression_type=%s", EXPRESSION_TYPES[mcf::enum_index(targetExpression->get_expression_type())]);
				if (test_literal(targetExpression, static_cast<const mcf::ast::literal_expession*>(testCases[i].ExpectedTargetExpression.get())->get_token()) == false)
				{
					return false;
				}
				break;
			case mcf::ast::expression_type::identifier: __COUNTER__;
				fatal_assert(testCases[i].ExpectedTargetExpression->get_expression_type() == mcf::ast::expression_type::identifier,
					u8"targetExpression의 expression type은 identifier여야 합니다. expression_type=%s", EXPRESSION_TYPES[mcf::enum_index(targetExpression->get_expression_type())]);
				if (test_identifier(targetExpression, static_cast<const mcf::ast::identifier_expression*>(testCases[i].ExpectedTargetExpression.get())->get_token()) == false)
				{
					return false;
				}
				break;
			case mcf::ast::expression_type::data_type: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::prefix: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::infix: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::index_unknown: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::index: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::enum_block: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::enum_value_increment: __COUNTER__; [[fallthrough]];
			default:
				fatal_error(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. expression_type=%s(%zu)",
					EXPRESSION_TYPES[mcf::enum_index(targetExpression->get_expression_type())], mcf::enum_index(targetExpression->get_expression_type()));
			}
			constexpr const size_t TARGET_EXPRESSION_TYPE_COUNT = __COUNTER__ - TARGET_EXPRESSION_TYPE_COUNT_BEGIN;
			static_assert(static_cast<size_t>(mcf::ast::expression_type::count) == TARGET_EXPRESSION_TYPE_COUNT, "expression_type count is changed. this SWITCH need to be changed as well.");
		}

		return true;
		});

	_names.emplace_back("test_infix_expressions");
	_tests.emplace_back([&]() {
		const struct test_case
		{
			const std::string	Input;
			const mcf::token	ExpectedLeftToken;
			const mcf::token	ExpectedInfixToken;
			const mcf::token	ExpectedRightToken;
		} testCases[] =
		{
			{"int32 foo = 5 + 5;", { mcf::token_type::integer, "5" }, { mcf::token_type::plus, "+" }, { mcf::token_type::integer, "5" }},
			{"int32 foo = 5 - 5;", { mcf::token_type::integer, "5" }, { mcf::token_type::minus, "-" }, { mcf::token_type::integer, "5" }},
			{"int32 foo = 5 * 5;", { mcf::token_type::integer, "5" }, { mcf::token_type::asterisk, "*" }, { mcf::token_type::integer, "5" }},
			{"int32 foo = 5 / 5;", { mcf::token_type::integer, "5" }, { mcf::token_type::slash, "/" }, { mcf::token_type::integer, "5" }},
			//TODO: {"int32 foo = 5 > 5;", { mcf::token_type::integer, "5" }, { mcf::token_type::greater, ">" }, { mcf::token_type::integer, "5" }},
			//TODO: {"int32 foo = 5 < 5;", { mcf::token_type::integer, "5" }, { mcf::token_type::less, "<" }, { mcf::token_type::integer, "5" }},
			//TODO: {"int32 foo = 5 == 5;", { mcf::token_type::integer, "5" }, { mcf::token_type::equal, "==" }, { mcf::token_type::integer, "5" }},
			//TODO: {"int32 foo = 5 != 5;", { mcf::token_type::integer, "5" }, { mcf::token_type::not_equal, "!=" }, { mcf::token_type::integer, "5" }},
		};
		constexpr const size_t testCaseCount = array_size(testCases);

		for (size_t i = 0; i < testCaseCount; i++)
		{
			mcf::parser parser(testCases[i].Input, false);
			mcf::ast::program program;
			parser.parse_program(program);
			if (check_parser_errors(parser) == false)
			{
				return false;
			}

			fatal_assert(program.get_statement_count() == 1, u8"program._statements 안에 1개의 명령문을 가지고 있어야 합니다. 결과값=%zu", program.get_statement_count());
			const mcf::ast::statement* statement = program.get_statement_at(0);

			fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable, u8"statement의 statement type이 variable가 아닙니다. statement=%s",
				STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);
			const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_statement*>(statement)->get_init_expression();

			fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::infix, u8"init expression의 expression type이 infix가 아닙니다. expression_type=%s",
				EXPRESSION_TYPES[mcf::enum_index(initExpression->get_expression_type())]);
			const mcf::ast::infix_expression* infixExpression = static_cast<const mcf::ast::infix_expression*>(initExpression);

			const mcf::ast::expression* leftExpression = infixExpression->get_left_expression();
			constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
			switch (leftExpression->get_expression_type())
			{
			case mcf::ast::expression_type::literal: __COUNTER__;
				if (test_literal(leftExpression, testCases[i].ExpectedLeftToken) == false)
				{
					return false;
				}
				break;
			case mcf::ast::expression_type::identifier: __COUNTER__;
				if (test_identifier(leftExpression, testCases[i].ExpectedLeftToken) == false)
				{
					return false;
				}
				break;
			case mcf::ast::expression_type::data_type: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::prefix: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::infix: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::index_unknown: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::index: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::enum_block: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::enum_value_increment: __COUNTER__; [[fallthrough]];
			default:
				fatal_error(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. left expression_type=%s(%zu)",
					EXPRESSION_TYPES[mcf::enum_index(leftExpression->get_expression_type())], mcf::enum_index(leftExpression->get_expression_type()));
			}
			constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT = __COUNTER__ - LEFT_EXPRESSION_TYPE_COUNT_BEGIN;
			static_assert(static_cast<size_t>(mcf::ast::expression_type::count) == LEFT_EXPRESSION_TYPE_COUNT, "expression_type count is changed. this SWITCH need to be changed as well.");

			fatal_assert(infixExpression->get_infix_operator_token() == testCases[i].ExpectedInfixToken, u8"infix operator token이 %s와 다릅니다. token=%s",
				convert_to_string(testCases[i].ExpectedInfixToken).c_str(), convert_to_string(infixExpression->get_infix_operator_token()).c_str());

			const mcf::ast::expression* rightExpression = infixExpression->get_left_expression();
			constexpr const size_t RIGHT_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
			switch (rightExpression->get_expression_type())
			{
			case mcf::ast::expression_type::literal: __COUNTER__;
				if (test_literal(rightExpression, testCases[i].ExpectedLeftToken) == false)
				{
					return false;
				}
				break;
			case mcf::ast::expression_type::identifier: __COUNTER__;
				if (test_identifier(rightExpression, testCases[i].ExpectedLeftToken) == false)
				{
					return false;
				}
				break;
			case mcf::ast::expression_type::data_type: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::prefix: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::infix: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::index_unknown: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::index: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::enum_block: __COUNTER__; [[fallthrough]];
			case mcf::ast::expression_type::enum_value_increment: __COUNTER__; [[fallthrough]];
			default:
				fatal_error(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. right expression_type=%s(%zu)",
					EXPRESSION_TYPES[mcf::enum_index(rightExpression->get_expression_type())], mcf::enum_index(rightExpression->get_expression_type()));
			}
			constexpr const size_t RIGHT_EXPRESSION_TYPE_COUNT = __COUNTER__ - RIGHT_EXPRESSION_TYPE_COUNT_BEGIN;
			static_assert(static_cast<size_t>(mcf::ast::expression_type::count) == RIGHT_EXPRESSION_TYPE_COUNT, "expression_type count is changed. this SWITCH need to be changed as well.");
		}

		return true;
		});

	_names.emplace_back("test_operator_precedence");
	_tests.emplace_back([&]() {
		const struct test_case
		{
			const std::string	Input;
			const std::string	Expected;
		} testCases[] =
		{
			// TODO: 테스트 통과를 위해 필요한 기능 구현
			{
				"int32 test = const int32;",
				"int32 test = const int32;",
			},
			/*{
				"int32 test = (int32)a;",
				"int32 test = ((int32)a);",
			},
			{
				"int32 test = int32(0);",
				"int32 test = (int32(0));",
			},*/
			/*{
				"int32 test = (int32)a;",
				"int32 test = ((int32)a);",
			},*/
			{
				"const int32 test = -a * b;",
				"const int32 test = ((-a) * b);",
			},
			/*{
				"int32 test = !-a;",
				"int32 test = (!(-a));",
			},*/
			{
				"int32 test = a + b - c;",
				"int32 test = ((a + b) - c);",
			},
			{
				"int32 test = a * b * c;",
				"int32 test = ((a * b) * c);",
			},
			{
				"int32 test = a * b / c;",
				"int32 test = ((a * b) / c);",
			},
			{
				"int32 test = a + b / c;",
				"int32 test = (a + (b / c));",
			},
			{
				"int32 test = a + b * c + d / e - f;",
				"int32 test = (((a + (b * c)) + (d / e)) - f);",
			},
			{
				"int32 test = 3 + 4; int32 test2 = -5 * 5;",
				"int32 test = (3 + 4);int32 test2 = ((-5) * 5);",
			},
			/*{
				"int32 test = 5 > 4 == 3 < 4;",
				"int32 test = ((5 > 4) == (3 < 4));",
			},
			{
				"int32 test = 5 < 4 != 3 > 4;",
				"int32 test = ((5 < 4) != (3 > 4));",
			},
			{
				"int32 test = 3 + 4 * 5 == 3 * 1 + 4 * 5;",
				"int32 test = ((3 + (4 * 5)) == ((3 * 1) + (4 * 5)));",
			},*/
			{
				"test = -a * b;",
				"test = ((-a) * b);",
			},
			/*{
				"test = !-a;",
				"test = (!(-a));",
			},*/
			{
				"test = a + b - c;",
				"test = ((a + b) - c);",
			},
			{
				"test = a * b * c;",
				"test = ((a * b) * c);",
			},
			{
				"test = a * b / c;",
				"test = ((a * b) / c);",
			},
			{
				"test = a + b / c;",
				"test = (a + (b / c));",
			},
			{
				"test = a + b * c + d / e - f;",
				"test = (((a + (b * c)) + (d / e)) - f);",
			},
			{
				"int32 test = 3 + 4; test = -5 * 5;",
				"int32 test = (3 + 4);test = ((-5) * 5);",
			},
			/*{
				"test = 5 > 4 == 3 < 4;",
				"test = ((5 > 4) == (3 < 4));",
			},
			{
				"test = 5 < 4 != 3 > 4;",
				"test = ((5 < 4) != (3 > 4));",
			},
			{
				"test = 3 + 4 * 5 == 3 * 1 + 4 * 5;",
				"test = ((3 + (4 * 5)) == ((3 * 1) + (4 * 5)));",
			},*/
		};
		constexpr const size_t testCaseCount = array_size(testCases);

		for (size_t i = 0; i < testCaseCount; i++)
		{
			mcf::parser parser(testCases[i].Input, false);
			mcf::ast::program program;
			parser.parse_program(program);
			if (check_parser_errors(parser) == false)
			{
				return false;
			}

			const std::string actual = program.convert_to_string();
			fatal_assert(actual == testCases[i].Expected, "expected=`%s`, actual=`%s`", testCases[i].Expected.c_str(), actual.c_str());
		}

		return true;
		});

	constexpr const char* const test_file_read = "./test/unittest/texts/test_file_read.txt";
	_names.emplace_back(test_file_read);
	_tests.emplace_back([&]() {
		mcf::ast::program actualProgram;
		{
			mcf::parser parser(test_file_read, true);
			mcf::parser::error parserInitError = parser.get_last_error();
			fatal_assert(parserInitError.ID == mcf::parser::error::id::no_error, "ID=`%s`, File=`%s`(%zu, %zu)\n%s",
				PARSER_ERROR_ID[enum_index(parserInitError.ID)], parserInitError.Name.c_str(), parserInitError.Line, parserInitError.Index, parserInitError.Message.c_str());
			parser.parse_program(actualProgram);
			if (check_parser_errors(parser) == false)
			{
				return false;
			}
		}

		using namespace mcf;
		using namespace mcf::ast;

		auto LiteralVariableStatement = [&](data_type_expression type, const char* name, literal_expession* literalExpression) -> mcf::ast::statement* {
			return new variable_statement(type, Identifier(name), literalExpression);
			};

		auto EnumStatement = [](data_type_expression enumDataType, const char* enumName, std::initializer_list<const char*> valueNames)
			{
				auto BuildBlockStatement = [](std::initializer_list<const char*> names) -> enum_block_statements_expression* {
					auto EnumValueNames = [](std::initializer_list<const char*> names) -> std::vector<identifier_expression> {
						std::vector<identifier_expression> nameVector;
						const char* const* pointer = names.begin();
						for (size_t i = 0; i < names.size(); i++)
						{
							nameVector.emplace_back(token{ token_type::identifier, pointer[i]});
						}
						return nameVector;
						};
					auto EnumIncrementValues = [](size_t length) -> enum_block_statements_expression::unique_expression* {
						enum_block_statements_expression::unique_expression* values = new enum_block_statements_expression::unique_expression[length];
						for (size_t i = 0; i < length; i++)
						{
							values[i] = std::unique_ptr<const mcf::ast::expression>(new enum_value_increment());
						}
						return values;
						};
					auto nameVector = EnumValueNames(names);
					return new enum_block_statements_expression(nameVector, EnumIncrementValues(names.size()));
					};
				return new enum_statement(enumDataType, Identifier(enumName), BuildBlockStatement(valueNames));
			};

		auto newInt = [](int32_t value) -> literal_expession* { return new literal_expession({ token_type::integer, std::to_string(value) }); };

		mcf::ast::statement* statements[] =
		{
			// int32 foo = 10; 
			LiteralVariableStatement(type_int32, "foo", newInt(10)),
			// int32 boo = 5;							
			LiteralVariableStatement(type_int32, "boo", newInt(5)),
			// enum PRINT_RESULT : int32{ NO_ERROR, };
			EnumStatement(type_uint8, "PRINT_RESULT", {"NO_ERROR"}),
		};
		size_t statementSize = array_size(statements);

		mcf::ast::statement_array statementArray = std::make_unique<mcf::ast::unique_statement[]>(statementSize);
		for (size_t i = 0; i < statementSize; i++)
		{
			statementArray[i] = mcf::ast::unique_statement(statements[i]);
		}

		program expectedProgram(statementArray.release(), statementSize);
		fatal_assert(actualProgram.convert_to_string() == expectedProgram.convert_to_string(), u8"생성된 문자열이 기대값과 다릅니다.\nactual:%s\nexpected:%s\n",
			actualProgram.convert_to_string().c_str(), expectedProgram.convert_to_string().c_str());

		return true;
		});

	_names.emplace_back("converting to tokens and compare");
	_tests.emplace_back([&]() {
		const struct test_case
		{
			const std::string				Input;
			const std::vector<mcf::token>	Expected;
		} testCases[] =
		{
			// TODO: 테스트 통과를 위해 필요한 기능 구현
			{
				"int32 test = 10;",
				{
					{mcf::token_type::keyword_int32, "int32"},
					{mcf::token_type::identifier, "test"},
					{mcf::token_type::assign, "="},
					{mcf::token_type::integer, "10"},
				},
			},
		};
		constexpr const size_t testCaseCount = array_size(testCases);

		for (size_t i = 0; i < testCaseCount; i++)
		{
			mcf::parser parser(testCases[i].Input, false);
			mcf::ast::program program;
			parser.parse_program(program);
			if (check_parser_errors(parser) == false)
			{
				return false;
			}

			const std::vector<mcf::token> actual = program.convert_to_tokens();
			fatal_assert(actual.size() == testCases[i].Expected.size(), u8"실제 토큰의 갯수와 테스트 케이스의 토큰 갯수가 맞지 않습니다. expected=`%zu`, actual=`%zu`",
				testCases[i].Expected.size(), actual.size());
			for (size_t j = 0; j < testCases[i].Expected.size(); j++)
			{
				fatal_assert(actual[j].Type == testCases[i].Expected[j].Type, u8"tests[line: %zu, index: %zu] - 토큰 타입이 틀렸습니다. 예상값=%s, 실제값=%s",
					actual[i].Line, actual[i].Index, TOKEN_TYPES[enum_index(testCases[i].Expected[j].Type)], TOKEN_TYPES[enum_index(actual[j].Type)]);

				fatal_assert(actual[j].Literal == testCases[i].Expected[j].Literal, u8"tests[line: %zu, index: %zu] - 토큰 리터럴이 틀렸습니다. 예상값=%s, 실제값=%s",
					actual[j].Line, actual[j].Index, testCases[i].Expected[j].Literal.c_str(), actual[j].Literal.c_str());
			}
		}

		return true;
		});
	_tests.pop_back();
}

const bool UnitTest::Parser::Test(void) const noexcept
{
	for (size_t i = 0; i < _tests.size(); i++)
	{
		if (_tests[i]() == false)
		{
			std::cout << "Test[#" << i << "] `" << _names[i] << "` Failed" << std::endl;
			return false;
		}
		std::cout << "Parser Test[#" << i << "] `" << _names[i] << "` Passed" << std::endl;
	}
	return true;
}

bool UnitTest::Parser::check_parser_errors(mcf::parser& parser) noexcept
{
	const size_t errorCount = parser.get_error_count();
	if (errorCount == 0)
	{
		return true;
	}

	error_message_begin(errorCount);
	mcf::parser::error curr = parser.get_last_error();
	while (curr.ID != mcf::parser::error::id::no_error)
	{
		error_message("[ID:%zu]%s%s", enum_index(curr.ID), curr.Name.c_str(), curr.Message.c_str());
		curr = parser.get_last_error();
	}
	error_message_end;
	return false;
}

bool UnitTest::Parser::test_variable_declaration_statement(const mcf::ast::statement* statement, const mcf::token_type expectedDataType, const std::string& expectedName) noexcept
{
	fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable, u8"statement가 variable이 아닙니다. 결과값=%s",
		STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);

	const mcf::ast::variable_statement* variableDeclaration = static_cast<const mcf::ast::variable_statement*>(statement);
	fatal_assert(variableDeclaration->get_type() == expectedDataType, u8"변수 선언 타입이 '%s'가 아닙니다. 실제값=%s",
		TOKEN_TYPES[enum_index(expectedDataType)], TOKEN_TYPES[enum_index(variableDeclaration->get_type())]);
	fatal_assert(variableDeclaration->get_name() == expectedName, u8"변수 선언 이름이 '%s'가 아닙니다. 실제값=%s", expectedName.c_str(), variableDeclaration->get_name().c_str());
	// TODO: #11 initialization 도 구현 필요

	return true;
}

bool UnitTest::Parser::test_literal(const mcf::ast::expression* expression, const mcf::token& expectedToken) noexcept
{
	fatal_assert(expression->get_expression_type() == mcf::ast::expression_type::literal, u8"expression의 expression type이 literal이 아닙니다. expression_type=%s",
		EXPRESSION_TYPES[mcf::enum_index(expression->get_expression_type())]);
	const mcf::ast::literal_expession* literalExpression = static_cast<const mcf::ast::literal_expession*>(expression);

	fatal_assert(literalExpression->get_token() == expectedToken, u8"literalExpression의 token이 %s이 아닙니다. expression_type=%s",
		convert_to_string(expectedToken).c_str(), convert_to_string(literalExpression->get_token()).c_str());
	return true;
}

bool UnitTest::Parser::test_identifier(const mcf::ast::expression* targetExpression, const mcf::token& expectedToken) noexcept
{
	fatal_assert(targetExpression->get_expression_type() == mcf::ast::expression_type::identifier, u8"expression의 expression type이 identifier이 아닙니다. expression_type=%s",
		EXPRESSION_TYPES[mcf::enum_index(targetExpression->get_expression_type())]);
	const mcf::ast::identifier_expression* identifierExpression = static_cast<const mcf::ast::identifier_expression*>(targetExpression);

	fatal_assert(identifierExpression->get_token() == expectedToken, u8"identifierExpression의 token이 %s이 아닙니다. expression_type=%s",
		convert_to_string(expectedToken).c_str(), convert_to_string(identifierExpression->get_token()).c_str());
	return true;
}