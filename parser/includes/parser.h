#pragma once
#include <stack>
#include <initializer_list>

namespace mcf
{
	class evaluator;

	namespace ast
	{
		class mcf::ast::statement;
	}

	enum class parser_error_id : unsigned char
	{
		invalid = 0,

		no_error,
		invalid_lexer_error_token,
		invalid_input_length,
		fail_read_file,
		fail_memory_allocation,
		fail_expression_parsing,
		fail_statement_parsing,
		unexpected_current_token,
		unexpected_next_token,
		not_registered_statement_token,
		not_registered_expression_token,
		not_registered_infix_expression_token,
		registering_duplicated_symbol_name,

		// 이 밑으로는 수정하면 안됩니다.
		count,
	};

	struct parser_error final
	{
		parser_error_id ID;
		std::string		Name;
		std::string		Message;
		size_t			Line;
		size_t			Index;
	};

	class parser final
	{
	public:
		enum class precedence : unsigned char
		{
			invalid = 0,

			lowest,
			equals,			// == | !=
			lessgreater,	// > | <
			sum,			// + | -
			product,		// * | /
			prefix,			// -foo | !boo
			call,			// func(x)
			index,			// foo[0]

			// 이 밑으로는 수정하면 안됩니다.
			count
		};

	public:
		explicit parser(void) noexcept = delete;
		explicit parser(evaluator* evaluator, const std::string& input, const bool isFile) noexcept;

		const size_t				get_error_count(void) noexcept;
		const mcf::parser_error		get_last_error(void) noexcept;

		void parse_program( ast::program& outProgram ) noexcept;

	private:
		std::unique_ptr<const mcf::ast::statement>		parse_statement(void) noexcept;
		std::unique_ptr<const mcf::ast::statement>		parse_declaration_statement(void) noexcept;
		std::unique_ptr<const mcf::ast::statement>		parse_call_or_assign_statement(void) noexcept;
		std::unique_ptr<const mcf::ast::enum_statement> parse_enum_statement(void) noexcept;

		std::unique_ptr<const mcf::ast::expression>							parse_expression(const mcf::parser::precedence precedence) noexcept;
		std::unique_ptr<const mcf::ast::expression>							parse_data_type_or_identifier_expressions(void) noexcept;
		std::unique_ptr<const mcf::ast::prefix_expression>					parse_prefix_expression(void) noexcept;
		std::unique_ptr<const mcf::ast::infix_expression>					parse_infix_expression(std::unique_ptr<const mcf::ast::expression>&& left) noexcept;
		std::unique_ptr<const mcf::ast::function_call_expression>			parse_call_expression(std::unique_ptr<const mcf::ast::expression>&& left) noexcept;
		std::unique_ptr<const mcf::ast::index_expression>					parse_index_expression(std::unique_ptr<const mcf::ast::expression>&& left) noexcept;
		std::unique_ptr<const mcf::ast::function_parameter_list_expression>	parse_function_parameters(void) noexcept;
		std::unique_ptr<const mcf::ast::function_block_expression>			parse_function_block_expression(void) noexcept;
		std::unique_ptr<const mcf::ast::enum_block_expression>				parse_enum_block_expression(void) noexcept;

		void		read_next_token(void) noexcept;
		const bool	read_next_token_if(mcf::token_type tokenType) noexcept;
		const bool	read_next_token_if(mcf::token_type tokenType, const std::string& scopeToPush) noexcept;
		const bool	read_next_token_if_any(std::initializer_list<token_type> list) noexcept;

		const bool	is_token_data_type(const mcf::token& token) const noexcept;

		constexpr const std::initializer_list<mcf::token_type> get_data_type_list(void) const noexcept;


		const mcf::parser::precedence get_infix_expression_token_precedence(const mcf::token& token) noexcept;
		const mcf::parser::precedence get_next_infix_expression_token_precedence( void ) noexcept;
		const mcf::parser::precedence get_current_infix_expression_token_precedence(void) noexcept;

		const bool check_last_lexer_error(void) noexcept;

	private:
		evaluator* _evaluator = nullptr;

		std::stack<mcf::parser_error>	_errors;

		mcf::lexer _lexer;
		mcf::token _currentToken;
		mcf::token _nextToken;
	};

	constexpr const char* PARSER_ERROR_ID[] =
	{
		"invalid",

		"no_error",
		"invalid_lexer_error_token",
		"invalid_input_length",
		"fail_read_file",
		"fail_memory_allocation",
		"fail_expression_parsing",
		"fail_statement_parsing",
		"unexpected_current_token",
		"unexpected_next_token",
		"not_registered_statement_token",
		"not_registered_expression_token",
		"not_registered_infix_expression_token",
		"registering_duplicated_symbol_name",
	};
	constexpr const size_t PARSER_ERROR_ID_SIZE = array_size( PARSER_ERROR_ID );
	static_assert(static_cast<size_t>(mcf::parser_error_id::count) == PARSER_ERROR_ID_SIZE, "mcf::parser::error::id count not matching");
}