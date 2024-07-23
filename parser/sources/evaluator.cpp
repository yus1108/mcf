#include "pch.h"
#include "evaluator.h"
#include "framework.h"

const mcf::token_type mcf::evaluator::register_custom_enum_type(std::string name) noexcept
{
	debug_assert(_scope.empty() == false, "");

	if (is_datatype_registered_at_current_scope(name) == true)
	{
		return token_type::invalid;
	}

	mcf::token_type tokenType = token_type::custom_enum_type;
	std::string scopeString;
	for (size_t i = 0; i < _scope.size(); i++)
	{
		scopeString = (i == 0 ? "" : "::") + _scope[i];
	}
	_customDataTypeMap[scopeString + "::" + name] = tokenType;
	return tokenType;
}

const mcf::token_type mcf::evaluator::find_datatype_registered(const std::string& tokenLiteral) const noexcept
{
	const mcf::token_type tokenFound = find_keyword_token_type(tokenLiteral);
	if (tokenFound != token_type::invalid)
		return tokenFound;

	// 주어진 모든 스코프들 안에서 검색 하고 없는 경우 identifier 반환
	std::vector<std::string> scope = _scope;
	while (scope.empty() == false)
	{
		std::string scopeString;
		for (size_t i = 0; i < scope.size(); i++)
		{
			scopeString = (i == 0 ? "" : "::") + scope[i];
		}
		auto iter = _customDataTypeMap.find(scopeString + "::" + tokenLiteral);
		if (iter != _customDataTypeMap.end())
		{
			return iter->second;
		}
		scope.pop_back();
	}
	return mcf::token_type::invalid;
}

const bool mcf::evaluator::is_datatype_registered_at_current_scope(const std::string& tokenLiteral) const noexcept
{
	const mcf::token_type tokenFound = find_keyword_token_type(tokenLiteral);
	if (tokenFound != token_type::invalid)
		return true;

	// 특정 스코프에서만 검색 하고 없는 경우 false 반환
	std::string scopeString = convert_scope_to_string();
	auto iter = _customDataTypeMap.find(scopeString + "::" + tokenLiteral);
	return iter != _customDataTypeMap.end();
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

void mcf::evaluator::push(std::string scope) noexcept
{
	_scope.emplace_back(scope);
}

void mcf::evaluator::pop(void) noexcept
{
	_scope.pop_back();
}

const std::string mcf::evaluator::convert_scope_to_string(void) const noexcept
{
	std::string scopeString;
	for (size_t i = 0; i < _scope.size(); i++)
	{
		scopeString = (i == 0 ? "" : "::") + _scope[i];
	}
	return scopeString;
}
