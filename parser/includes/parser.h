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
		struct error final
		{
			enum class id : unsigned char
			{
				no_error = 0,
				unexpected_next_token,

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
		const mcf::ast::variable_declaration_statement* parse_variable_declaration_statement() noexcept;

		void		read_next_toekn(void) noexcept;
		const bool	read_next_token_if(mcf::token_type tokenType) noexcept;

	private:
		mcf::lexer _lexer;
		mcf::token _currentToken;
		mcf::token _nextToken;
	};
}