#pragma once

namespace mcf
{
	namespace ast
	{
		class Program;
	}

	class parser final
	{
	public:
		explicit parser(void) noexcept = delete;
		explicit parser(const std::string& input) noexcept;

		mcf::ast::program* parse_program( void ) noexcept;

	private:
		inline void read_next_token(void) noexcept;

	private:
		mcf::lexer _lexer;
		mcf::token _currentToken;
		mcf::token _nextToken;
	};
}