#pragma once
#include <vector>
#include <filesystem>
#include <unordered_set>
#include "lexer.h"

namespace mcf
{
	namespace ast
	{
		class program;
	}

	class evaluator final
	{
	public:
		const mcf::token_type register_custom_enum_type(std::string name) noexcept;
		const mcf::token_type find_datatype_registered(const std::string& tokenLiteral) const noexcept;

		const bool is_datatype_registered_at_current_scope(const std::string& tokenLiteral) const noexcept;
		const bool include_project_file(const std::string& filePath) noexcept;

		void push(std::string scope) noexcept;
		void pop(void) noexcept;

		const std::string convert_scope_to_string(void) const noexcept;

	private:
		std::unordered_set<std::filesystem::path::string_type>	_includedProjectFIles;
		std::unordered_map<std::string, token_type>				_customDataTypeMap;
		std::vector<std::string> _scope = std::vector<std::string>({ "global" });
	};
}