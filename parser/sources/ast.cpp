#include "pch.h"
#include "ast.h"

mcf::ast::variable_declaration_statement::variable_declaration_statement(	
	const mcf::ast::data_type_expression& dataType, 
	const mcf::ast::identifier_expression& name, 
	const mcf::ast::expression* rightExpression) noexcept
	: _dataType(dataType)
	, _name(name)
	, _rightExpression(rightExpression)
{
}

const std::string mcf::ast::variable_declaration_statement::convert_to_string(void) const noexcept
{
	std::string buffer;

	buffer += _dataType.convert_to_string();
	buffer += " " + _name.convert_to_string();
	if (_rightExpression.get() != nullptr)
	{
		buffer += " = " + _rightExpression->convert_to_string();
	}
	buffer += ";";

	return buffer;
}

mcf::ast::expression_statement::expression_statement(mcf::token token, const mcf::ast::expression* expression) noexcept
	: _token(token)
	, _expression(expression)
{
}

const std::string mcf::ast::expression_statement::convert_to_string(void) const noexcept
{
	// TODO: #14 assert for _expression == nullptr

	std::string buffer;

	buffer += _expression->convert_to_string();
	buffer += ";";

	return buffer;
}

mcf::ast::program::program(std::vector<const mcf::ast::statement*> statements) noexcept
{
	_count = statements.size();
	_statements = std::make_unique<unique_statement[]>(_count);
	for (size_t i = 0; i < _count; i++)
	{
		_statements[i] = unique_statement(statements[i]);
	}
}

const mcf::ast::statement* mcf::ast::program::get_statement_at(const size_t index) const noexcept
{
	return _statements[index].get();
}

const std::string mcf::ast::program::convert_to_string(void) const noexcept
{
	std::string buffer;

	for (size_t i = 0; i < _count; i++)
	{
		// TODO: #14 assert for _statements[i] == nullptr
		buffer += _statements[i]->convert_to_string() + "\n";
	}

	return buffer;
}
