#pragma once
#include <vector>
#include "lexer.h"

namespace mcf
{
	class evaluator final
	{
	public:
		const mcf::token_type register_custom_enum_type(std::string name) noexcept;
		const mcf::token_type find_datatype_registered(const std::string& tokenLiteral) const noexcept;

		const bool is_datatype_registered_at_current_scope(const std::string& tokenLiteral) const noexcept;

		void push(std::string scope) noexcept;
		void pop(void) noexcept;

		const std::string convert_scope_to_string(void) const noexcept;

	private:
		std::unordered_map<std::string, token_type> _customDataTypeMap;
		std::vector<std::string>					_scope = std::vector<std::string>({ "global" });
	};
}