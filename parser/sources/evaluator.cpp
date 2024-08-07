#include "ast.h"
#include "pch.h"
#include "evaluator.h"
#include "framework.h"

namespace mcf
{
	namespace internal
	{
		inline static const std::string convert_scope_to_string(const std::vector<std::string>& scope) noexcept
		{
			std::string scopeString;
			const size_t scopeCount = scope.size();
			for ( size_t i = 0; i < scopeCount; i++ )
			{
				scopeString += (i == 0 ? "" : "::") + scope[i];
			}
			return scopeString;
		}
	}
}

const mcf::token mcf::evaluator::find_keyword(const std::string& tokenLiteral) const noexcept
{
	const mcf::token tokenFound = find_predefined_keyword(tokenLiteral);
	if (tokenFound.Type != token_type::invalid)
		return tokenFound;

	// 주어진 모든 스코프들 안에서 검색 하고 없는 경우 identifier 반환
	std::vector<std::string> scope = _scope;
	while (scope.empty() == false)
	{
		std::string scopeString = internal::convert_scope_to_string(scope);
		auto iter = _customKeywordMap.find(scopeString + "::" + tokenLiteral);
		if (iter != _customKeywordMap.end())
		{
			return iter->second;
		}
		scope.pop_back();
	}
	return mcf::token{ mcf::token_type::invalid, "invalid" };
}

const bool mcf::evaluator::has_keyword_at_current_scope(const std::string& tokenLiteral) const noexcept
{
	const mcf::token_type tokenFound = find_predefined_keyword(tokenLiteral).Type;
	if (tokenFound != token_type::invalid)
		return true;

	// 특정 스코프에서만 검색 하고 없는 경우 false 반환
	std::string scopeString = convert_scope_to_string();
	auto iter = _customKeywordMap.find(scopeString + "::" + tokenLiteral);
	return iter != _customKeywordMap.end();
}

const mcf::token mcf::evaluator::register_custom_enum_type(const mcf::token originalToken) noexcept
{
	if (has_keyword_at_current_scope(originalToken.Literal) == true)
	{
		return token{ token_type::invalid, "invalid" };
	}

	mcf::token newToken = originalToken;
	newToken.Type = token_type::custom_enum_type;
	std::string scopeString = internal::convert_scope_to_string(_scope);
	_customKeywordMap[scopeString + "::" + originalToken.Literal] = newToken;
	return newToken;
}

void mcf::evaluator::register_custom_enum_value(const std::string value) noexcept
{
	debug_assert(has_keyword_at_current_scope(value) == false, u8"중복되는 value 값입니다.");
	std::string scopeString = internal::convert_scope_to_string(_scope);
	std::unordered_map<std::string, size_t>& valueMap = _enumValuesMap[scopeString];
	valueMap[value] = valueMap.size();
}

const bool mcf::evaluator::include_project_file(const std::string& filePath) noexcept
{
	std::filesystem::path path = filePath;
	std::filesystem::path absolutePath = std::filesystem::absolute(path);
	auto iter = _includedProjectFIles.find(absolutePath);
	if (iter != _includedProjectFIles.end())
	{
		return false;
	}
	_includedProjectFIles.insert(absolutePath);
	return true;
}

void mcf::evaluator::push_scope(std::string scope) noexcept
{
	_scope.emplace_back(scope);
}

void mcf::evaluator::pop_scope(void) noexcept
{
	_scope.pop_back();
}

const std::string mcf::evaluator::convert_scope_to_string(void) const noexcept
{
	return internal::convert_scope_to_string(_scope);
}
