#pragma once
namespace mcf
{
	namespace ast
	{
		class mcf::ast::statement;
	}

	class parser final
	{
	public:
		enum class precedence : unsigned char
		{
			invalid = 0,

			lowest,
			equals,			// == | !=		TODO: #16 구현 필요
			lessgreater,	// > | <		TODO: #16 구현 필요
			sum,			// + | -
			product,		// * | /
			prefix,			// -foo | !boo
			call,			// func(x)		TODO: #16 구현 필요
			index,			// foo[0]		TODO: #16 구현 필요

			// 이 밑으로는 수정하면 안됩니다.
			count
		};

		struct error final
		{
			enum class id : unsigned char
			{
				invalid = 0,

				no_error,
				unexpected_next_token,
				not_registered_prefix_token,
				not_registered_infix_token,

				// 이 밑으로는 수정하면 안됩니다.
				count,
			} ID;
			std::string Message;
		};

	public:
		explicit parser(void) noexcept = delete;
		explicit parser(const std::string& input) noexcept;

		static const size_t				get_error_count(void) noexcept;
		static const mcf::parser::error get_last_error(void) noexcept;

		void parse_program(mcf::ast::program& outProgram) noexcept;

	private:
		const mcf::ast::statement*						parse_statement(void) noexcept;
		const mcf::ast::variable_declaration_statement* parse_variable_declaration_statement(void) noexcept;
		const mcf::ast::variable_assignment_statement*	parse_variable_assignment_statement(void) noexcept;

		const mcf::ast::expression*			parse_expression(const mcf::parser::precedence precedence) noexcept;
		const mcf::ast::prefix_expression*	parse_prefix_expression(void) noexcept;
		const mcf::ast::infix_expression*	parse_infix_expression(const mcf::ast::expression* left) noexcept;
		const mcf::ast::infix_expression*	parse_call_expression(const mcf::ast::expression* left) noexcept;
		const mcf::ast::infix_expression*	parse_index_expression(const mcf::ast::expression* left) noexcept;

		void		read_next_token(void) noexcept;
		const bool	read_next_token_if(mcf::token_type tokenType) noexcept;

		const mcf::parser::precedence get_next_precedence(void) const noexcept;
		const mcf::parser::precedence get_current_token_precedence(void) const noexcept;

	private:
		mcf::lexer _lexer;
		mcf::token _currentToken;
		mcf::token _nextToken;
	};
}