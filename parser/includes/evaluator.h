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
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		//                                              KEYWORD                                             //
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		const mcf::token find_keyword( const std::string& tokenLiteral ) const noexcept;

		const bool has_keyword_at_current_scope(const std::string& tokenLiteral) const noexcept;

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		//                                               ENUM                                               //
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		const mcf::token		register_custom_enum_type(const mcf::token originalToken) noexcept;
		void					register_custom_enum_value(const std::string value) noexcept;

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		//                                              INCLUDE                                             //
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		const bool include_project_file(const std::string& filePath) noexcept;

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		//                                               SCOPE                                              //
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		void push_scope(std::string scope) noexcept;
		void pop_scope(void) noexcept;

		const std::string convert_scope_to_string(void) const noexcept;

	private:
		std::unordered_set<std::filesystem::path::string_type>	_includedProjectFIles;
		std::unordered_map<std::string, mcf::token>				_customKeywordMap;
		std::unordered_map<std::string, std::unordered_map<std::string, size_t>> _enumValuesMap;
		std::vector<std::string> _scope = std::vector<std::string>({ "global" });
	};
}