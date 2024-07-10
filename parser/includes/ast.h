#pragma once

#include <string>

namespace mcf
{
	namespace ast
	{
		class node
		{
		public: virtual const std::string get_token_literal(void) noexcept = 0;
		};

		class statement : public node
		{
		public: virtual const std::string get_token_literal(void) noexcept override = 0;
		};

		class expression : public node
		{
		public: virtual const std::string get_token_literal(void) noexcept override = 0;
		};
	}
}
