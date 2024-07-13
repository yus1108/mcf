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
		enum class expression_precedence : unsigned char
		{
			lowest = 0,
			equals,			// ==
			lessgreater,	// > | <
			sum,			// + | -
			product,		// * | /
			prefix,			// -foo | !boo
			call,			// func(x)
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
		const mcf::ast::statement* parse_statement(void) noexcept;
		const mcf::ast::variable_declaration_statement* parse_variable_declaration_statement(void) noexcept;

		const mcf::ast::expression* parse_expression(const mcf::parser::expression_precedence precedence) noexcept;

		void		read_next_toekn(void) noexcept;
		const bool	read_next_token_if(mcf::token_type tokenType) noexcept;

	private:
		mcf::lexer _lexer;
		mcf::token _currentToken;
		mcf::token _nextToken;
	};
}