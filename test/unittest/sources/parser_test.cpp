#include <iostream>
#include <fstream>
#include "test/unittest/unittest.h"

UnitTest::Parser::Parser(void) noexcept
{
	_names.emplace_back("test_variable_declaration_statements");
	_tests.emplace_back([&]() {
		// TODO: #11 initialization �� ���� �ʿ�
		const std::string input = "int32 x; int32 y = 10; int32 foobar = 838383;";

		mcf::parser parser = mcf::parser(input);
		mcf::ast::program program;
		parser.parse_program(program);
		const size_t statementCount = program.get_statement_count();

		if (check_parser_errors(parser) == false)
		{
			return false;
		}
		fatal_assert(statementCount == 3, u8"program._statements �ȿ� 3���� ��ɹ��� ������ �־�� �մϴ�. �����=%zu", statementCount);

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
		mcf::ast::data_type_expression dataType({ mcf::token_type::integer_32bit, "int32" });
		mcf::ast::identifier_expression name({ mcf::token_type::identifier, "myVar" });
		mcf::ast::identifier_expression* rightExpression = new(std::nothrow) mcf::ast::identifier_expression({ mcf::token_type::identifier, "anotherVar" });
		mcf::ast::variable_declaration_statement* variableDeclarationStatement = new(std::nothrow) mcf::ast::variable_declaration_statement(dataType, name, rightExpression);
		std::vector<const mcf::ast::statement*> statements =
		{
			variableDeclarationStatement,
		};

		mcf::ast::program program(statements);
		fatal_assert(program.convert_to_string() == "int32 myVar = anotherVar;\n", u8"program�� string ��ȯ�� Ʋ�Ƚ��ϴ�. ������=`%s`", program.convert_to_string().c_str());

		return true;
		});

	_names.emplace_back("test_identifier_expression");
	_tests.emplace_back([&]() {
		const std::string input = "int32 foo = bar;";
		mcf::parser parser(input);
		mcf::ast::program program;
		parser.parse_program(program);
		if (check_parser_errors(parser) == false)
		{
			return false;
		}

		fatal_assert(program.get_statement_count() == 1, u8"program._statements �ȿ� 1���� ��ɹ��� ������ �־�� �մϴ�. �����=%zu", program.get_statement_count());
		const mcf::ast::statement* statement = program.get_statement_at(0);

		fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable_declaration, u8"statement�� statement type�� variable_declaration�� �ƴմϴ�. statement=%s",
			STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);
		const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_declaration_statement*>(statement)->get_init_expression();

		fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::identifier, u8"init expression�� expression type�� identifier�� �ƴմϴ�. expression_type=%s",
			EXPRESSION_TYPES[mcf::enum_index(initExpression->get_expression_type())]);

		const mcf::ast::identifier_expression* identifier = static_cast<const mcf::ast::identifier_expression*>(initExpression);
		fatal_assert(identifier->get_token().Literal == "bar", u8"identifier�� literal���� `bar`�� �ƴմϴ�. identifier.literal=`%s`", identifier->get_token().Literal.c_str());

		return true;
		});

	_names.emplace_back("test_literal_expression");
	_tests.emplace_back([&]() {
		const std::string input = "int32 foo = 5;";
		mcf::parser parser(input);
		mcf::ast::program program;
		parser.parse_program(program);
		if (check_parser_errors(parser) == false)
		{
			return false;
		}

		fatal_assert(program.get_statement_count() == 1, u8"program._statements �ȿ� 1���� ��ɹ��� ������ �־�� �մϴ�. �����=%zu", program.get_statement_count());
		const mcf::ast::statement* statement = program.get_statement_at(0);

		fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable_declaration, u8"statement�� statement type�� variable_declaration�� �ƴմϴ�. statement=%s",
			STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);
		const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_declaration_statement*>(statement)->get_init_expression();

		fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::literal, u8"init expression�� expression type�� literal�� �ƴմϴ�. expression_type=%s",
			EXPRESSION_TYPES[mcf::enum_index(initExpression->get_expression_type())]);

		const mcf::ast::literal_expession* literal = static_cast<const mcf::ast::literal_expession*>(initExpression);
		fatal_assert(literal->get_token().Literal == "5", u8"literal�� literal���� `5`�� �ƴմϴ�. identifier.literal=`%s`", literal->get_token().Literal.c_str());

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
			{"int32 foo = -5;", { token_type::minus, "-" }, make_unique<const literal_expession>(token{ token_type::integer_32bit, "5" })},
			// TODO: {"int32 foo = !5;", { token_type::not, "!" }, make_unique<const literal_expession>(token{ token_type::integer_32bit, "5" })},
			{"int32 foo = -15;", { token_type::minus, "-" }, make_unique<const literal_expession>(token{ token_type::integer_32bit, "15" })},
			{"int32 foo = +5;", { token_type::plus, "+" }, make_unique<const literal_expession>(token{ token_type::integer_32bit, "5" })},
			{"int32 foo = +15;", { token_type::plus, "+" }, make_unique<const literal_expession>(token{ token_type::integer_32bit, "15" })},
		};
		constexpr const size_t testCaseCount = array_size(testCases);

		for (size_t i = 0; i < testCaseCount; i++)
		{
			mcf::parser parser(testCases[i].Input);
			mcf::ast::program program;
			parser.parse_program(program);
			if (check_parser_errors(parser) == false)
			{
				return false;
			}

			fatal_assert(program.get_statement_count() == 1, u8"program._statements �ȿ� 1���� ��ɹ��� ������ �־�� �մϴ�. �����=%zu", program.get_statement_count());
			const mcf::ast::statement* statement = program.get_statement_at(0);

			fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable_declaration, u8"statement�� statement type�� variable_declaration�� �ƴմϴ�. statement=%s",
				STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);
			const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_declaration_statement*>(statement)->get_init_expression();

			fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::prefix, u8"init expression�� expression type�� prefix�� �ƴմϴ�. expression_type=%s",
				EXPRESSION_TYPES[mcf::enum_index(initExpression->get_expression_type())]);

			const mcf::ast::prefix_expression* prefixExpression = static_cast<const mcf::ast::prefix_expression*>(initExpression);
			fatal_assert(prefixExpression->get_prefix_operator_token() == testCases[i].ExpectedPrefixToken, u8"prefix operator token�� %s�� �ٸ��ϴ�. token=%s",
				convert_to_string(testCases[i].ExpectedPrefixToken).c_str(), convert_to_string(prefixExpression->get_prefix_operator_token()).c_str());

			const mcf::ast::expression* targetExpression = prefixExpression->get_target_expression();

			constexpr const size_t TARGET_EXPRESSION_TYPE_COUNT_BEGIN = __COUNTER__;
			switch (targetExpression->get_expression_type())
			{
			case mcf::ast::expression_type::literal: __COUNTER__;
				fatal_assert(testCases[i].ExpectedTargetExpression->get_expression_type() == mcf::ast::expression_type::literal,
					u8"targetExpression�� expression type�� literal���� �մϴ�. expression_type=%s", EXPRESSION_TYPES[mcf::enum_index(targetExpression->get_expression_type())]);
				if (test_literal(targetExpression, static_cast<const mcf::ast::literal_expession*>(testCases[i].ExpectedTargetExpression.get())->get_token()) == false)
				{
					return false;
				}
				break;
			case mcf::ast::expression_type::identifier: __COUNTER__;
				fatal_assert(testCases[i].ExpectedTargetExpression->get_expression_type() == mcf::ast::expression_type::identifier,
					u8"targetExpression�� expression type�� identifier���� �մϴ�. expression_type=%s", EXPRESSION_TYPES[mcf::enum_index(targetExpression->get_expression_type())]);
				if (test_identifier(targetExpression, static_cast<const mcf::ast::identifier_expression*>(testCases[i].ExpectedTargetExpression.get())->get_token()) == false)
				{
					return false;
				}
				break;
			case mcf::ast::expression_type::data_type: __COUNTER__;
			case mcf::ast::expression_type::prefix: __COUNTER__;
			case mcf::ast::expression_type::infix: __COUNTER__;
			default:
				fatal_error(u8"����ġ ���� ���� ���Խ��ϴ�. Ȯ�� �� �ּ���. expression_type=%s(%zu)",
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
			{"int32 foo = 5 + 5;", { mcf::token_type::integer_32bit, "5" }, { mcf::token_type::plus, "+" }, { mcf::token_type::integer_32bit, "5" }},
			{"int32 foo = 5 - 5;", { mcf::token_type::integer_32bit, "5" }, { mcf::token_type::minus, "-" }, { mcf::token_type::integer_32bit, "5" }},
			{"int32 foo = 5 * 5;", { mcf::token_type::integer_32bit, "5" }, { mcf::token_type::asterisk, "*" }, { mcf::token_type::integer_32bit, "5" }},
			{"int32 foo = 5 / 5;", { mcf::token_type::integer_32bit, "5" }, { mcf::token_type::slash, "/" }, { mcf::token_type::integer_32bit, "5" }},
			//TODO: {"int32 foo = 5 > 5;", { mcf::token_type::integer_32bit, "5" }, { mcf::token_type::greater, ">" }, { mcf::token_type::integer_32bit, "5" }},
			//TODO: {"int32 foo = 5 < 5;", { mcf::token_type::integer_32bit, "5" }, { mcf::token_type::less, "<" }, { mcf::token_type::integer_32bit, "5" }},
			//TODO: {"int32 foo = 5 == 5;", { mcf::token_type::integer_32bit, "5" }, { mcf::token_type::equal, "==" }, { mcf::token_type::integer_32bit, "5" }},
			//TODO: {"int32 foo = 5 != 5;", { mcf::token_type::integer_32bit, "5" }, { mcf::token_type::not_equal, "!=" }, { mcf::token_type::integer_32bit, "5" }},
		};
		constexpr const size_t testCaseCount = array_size(testCases);

		for (size_t i = 0; i < testCaseCount; i++)
		{
			mcf::parser parser(testCases[i].Input);
			mcf::ast::program program;
			parser.parse_program(program);
			if (check_parser_errors(parser) == false)
			{
				return false;
			}

			fatal_assert(program.get_statement_count() == 1, u8"program._statements �ȿ� 1���� ��ɹ��� ������ �־�� �մϴ�. �����=%zu", program.get_statement_count());
			const mcf::ast::statement* statement = program.get_statement_at(0);

			fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable_declaration, u8"statement�� statement type�� variable_declaration�� �ƴմϴ�. statement=%s",
				STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);
			const mcf::ast::expression* initExpression = static_cast<const mcf::ast::variable_declaration_statement*>(statement)->get_init_expression();

			fatal_assert(initExpression->get_expression_type() == mcf::ast::expression_type::infix, u8"init expression�� expression type�� infix�� �ƴմϴ�. expression_type=%s",
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
			case mcf::ast::expression_type::data_type: __COUNTER__;
			case mcf::ast::expression_type::prefix: __COUNTER__;
			case mcf::ast::expression_type::infix: __COUNTER__;
			default:
				fatal_error(u8"����ġ ���� ���� ���Խ��ϴ�. Ȯ�� �� �ּ���. left expression_type=%s(%zu)",
					EXPRESSION_TYPES[mcf::enum_index(leftExpression->get_expression_type())], mcf::enum_index(leftExpression->get_expression_type()));
			}
			constexpr const size_t LEFT_EXPRESSION_TYPE_COUNT = __COUNTER__ - LEFT_EXPRESSION_TYPE_COUNT_BEGIN;
			static_assert(static_cast<size_t>(mcf::ast::expression_type::count) == LEFT_EXPRESSION_TYPE_COUNT, "expression_type count is changed. this SWITCH need to be changed as well.");

			fatal_assert(infixExpression->get_infix_operator_token() == testCases[i].ExpectedInfixToken, u8"infix operator token�� %s�� �ٸ��ϴ�. token=%s",
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
			case mcf::ast::expression_type::data_type: __COUNTER__;
			case mcf::ast::expression_type::prefix: __COUNTER__;
			case mcf::ast::expression_type::infix: __COUNTER__;
			default:
				fatal_error(u8"����ġ ���� ���� ���Խ��ϴ�. Ȯ�� �� �ּ���. right expression_type=%s(%zu)",
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
			{
				"int32 test = -a * b;",
				"int32 test = ((-a) * b);",
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
			mcf::parser parser(testCases[i].Input);
			mcf::ast::program program;
			parser.parse_program(program);
			if (check_parser_errors(parser) == false)
			{
				return false;
			}

			const std::string actual = program.convert_to_string(false);
			fatal_assert(actual == testCases[i].Expected, "expected=`%s`, actual=`%s`", testCases[i].Expected.c_str(), actual.c_str());
		}

		return true;
		});

	_names.emplace_back("./unittest/texts/test_file_read.txt");
	_tests.emplace_back([&]() {
		
		mcf::ast::program actualProgram;
		std::cout << "file read begin" << std::endl;
		{
			std::string input;
			{
				std::ifstream file(_names.back().c_str());
				std::string line;
				while (std::getline(file, line))
				{
					input += line + "\n";
					std::cout << input << std::endl;
				}
				input.erase(input.length() - 1);
			}
			mcf::parser parser(input);
			parser.parse_program(actualProgram);
			if (check_parser_errors(parser) == false)
			{
				return false;
			}
		}
		std::cout << "file read end" << std::endl;

		using namespace mcf;
		using namespace mcf::ast;

		auto generate_variable_declaration = [&](token_type t1, const char* l1, token_type t2, const char* l2, token_type t3, const char* l3) -> statement* {
			return new variable_declaration_statement(data_type_expression({ t1, l1 }), identifier_expression({ t2, l2 }), new literal_expession({ t3, l3 }));
			};

		std::vector<const mcf::ast::statement*> statements =
		{
			generate_variable_declaration(token_type::keyword_int32, "int32", token_type::identifier, "foo", token_type::integer_32bit, "10"),
			generate_variable_declaration(token_type::keyword_int32, "int32", token_type::identifier, "boo", token_type::integer_32bit, "5"),
		};
		program expectedProgram(statements);
		fatal_assert(actualProgram.convert_to_string() == expectedProgram.convert_to_string(), u8"������ ���ڿ��� ��밪�� �ٸ��ϴ�.\nactual:%s\nexpected:%s\n",
			actualProgram.convert_to_string(false).c_str(), expectedProgram.convert_to_string(false).c_str());

		return true;
		});
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
		error_message(u8"[ID:%zu], %s", enum_index(curr.ID), curr.Message.c_str());
		curr = parser.get_last_error();
	}
	error_message_end;
	return false;
}

bool UnitTest::Parser::test_variable_declaration_statement(const mcf::ast::statement* statement, const mcf::token_type expectedDataType, const std::string& expectedName) noexcept
{
	fatal_assert(statement->get_statement_type() == mcf::ast::statement_type::variable_declaration,
		u8"statement�� variable_declaration�� �ƴմϴ�. �����=%s",
		STATEMENT_TYPES[mcf::enum_index(statement->get_statement_type())]);

	const mcf::ast::variable_declaration_statement* variableDeclaration = static_cast<const mcf::ast::variable_declaration_statement*>(statement);
	fatal_assert(variableDeclaration->get_type() == expectedDataType, u8"���� ���� Ÿ���� '%s'�� �ƴմϴ�. ������=%s",
		TOKEN_TYPES[enum_index(expectedDataType)], TOKEN_TYPES[enum_index(variableDeclaration->get_type())]);
	fatal_assert(variableDeclaration->get_name() == expectedName, u8"���� ���� �̸��� '%s'�� �ƴմϴ�. ������=%s", expectedName.c_str(), variableDeclaration->get_name().c_str());
	// TODO: #11 initialization �� ���� �ʿ�

	return true;
}

bool UnitTest::Parser::test_literal(const mcf::ast::expression* expression, const mcf::token& expectedToken) noexcept
{
	fatal_assert(expression->get_expression_type() == mcf::ast::expression_type::literal, u8"expression�� expression type�� literal�� �ƴմϴ�. expression_type=%s",
		EXPRESSION_TYPES[mcf::enum_index(expression->get_expression_type())]);
	const mcf::ast::literal_expession* literalExpression = static_cast<const mcf::ast::literal_expession*>(expression);

	fatal_assert(literalExpression->get_token() == expectedToken, u8"literalExpression�� token�� %s�� �ƴմϴ�. expression_type=%s",
		convert_to_string(expectedToken).c_str(), convert_to_string(literalExpression->get_token()).c_str());
	return true;
}

bool UnitTest::Parser::test_identifier(const mcf::ast::expression* targetExpression, const mcf::token& expectedToken) noexcept
{
	fatal_assert(targetExpression->get_expression_type() == mcf::ast::expression_type::identifier, u8"expression�� expression type�� identifier�� �ƴմϴ�. expression_type=%s",
		EXPRESSION_TYPES[mcf::enum_index(targetExpression->get_expression_type())]);
	const mcf::ast::identifier_expression* identifierExpression = static_cast<const mcf::ast::identifier_expression*>(targetExpression);

	fatal_assert(identifierExpression->get_token() == expectedToken, u8"identifierExpression�� token�� %s�� �ƴմϴ�. expression_type=%s",
		convert_to_string(expectedToken).c_str(), convert_to_string(identifierExpression->get_token()).c_str());
	return true;
}