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
		explicit parser(void) noexcept = delete;
		explicit parser(const std::string& input) noexcept;

		void parse_program(std::vector<const mcf::ast::statement*>& outProgram) noexcept;

	private:
		const mcf::ast::variable_declaration_statement* parse_variable_declaration_statement() noexcept;

		void read_next_toekn(void) noexcept;

	private:
		mcf::lexer _lexer;
		mcf::token _currentToken;
		mcf::token _nextToken;
	};
}