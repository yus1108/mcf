#include "pch.h"
#include "ast.h"

mcf::ast::program::program(std::unique_ptr<const mcf::ast::statement>* statements, size_t count) noexcept
	: _statements(statements)
	, _count(count)
{}

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
		buffer += _statements[i]->convert_to_string();
	}

	return buffer;
}

const std::vector<mcf::token> mcf::ast::program::convert_to_tokens(void) const noexcept
{
	std::vector<token> tokens;
	for (size_t i = 0; i < _count; i++)
	{
		// TODO: #14 assert for _statements[i] == nullptr
		// TODO: node::convert_to_tokens() 구현 필요
		//tokens.emplace_back(_statements->convert_to_tokens());
	}
	return tokens;
}

const std::string mcf::ast::prefix_expression::convert_to_string(void) const noexcept
{
	return "(" + _prefixOperator.Literal + _targetExpression->convert_to_string() + ")";
}

mcf::ast::enum_block_statements_expression::enum_block_statements_expression(std::vector<mcf::ast::identifier_expression >& names, unique_expression* values) noexcept
	: _names(names)
	, _values(values)
{
}

const std::string mcf::ast::enum_block_statements_expression::convert_to_string(void) const noexcept
{
	std::string buffer;

	const size_t size = _names.size();
	for (size_t i = 0; i < size; i++)
	{
		// TODO: #14 assert for _assignExpression == nullptr
		buffer += _names[i].convert_to_string();
		buffer += _values[i]->get_expression_type() == expression_type::enum_value_increment ? "" : " = ";
		buffer += _values[i]->convert_to_string() + ", ";
	}
	buffer.erase(buffer.size() - 1);
	return buffer;
}

mcf::ast::variable_statement::variable_statement(	
	const mcf::ast::data_type_expression& dataType, 
	const mcf::ast::expression* name,
	const mcf::ast::expression* initExpression) noexcept
	: _dataType(dataType)
	, _name(name)
	, _initExpression(initExpression)
{
}

const std::string mcf::ast::variable_statement::get_name(void) const noexcept
{
	// TODO: #14 assert for _name[i] == nullptr
	const expression* curr = _name.get();
	while (curr->get_expression_type() == expression_type::index)
	{
		curr = static_cast<const index_expression*>(curr)->get_left_expression();
	}

	// TODO: #14 assert for curr->get_expression_type() == expression_type::identifier
	return static_cast<const identifier_expression*>(curr)->convert_to_string();
}

const std::string mcf::ast::variable_statement::convert_to_string(void) const noexcept
{
	std::string buffer;

	buffer += _dataType.convert_to_string();
	buffer += " " + _name->convert_to_string();
	if (_initExpression.get() != nullptr)
	{
		buffer += " = " + _initExpression->convert_to_string();
	}
	buffer += ";";

	return buffer;
}

const std::string mcf::ast::infix_expression::convert_to_string(void) const noexcept
{
	return "(" + _left->convert_to_string() + " " + _infixOperator.Literal + " " + _right->convert_to_string() + ")";
}

mcf::ast::variable_assign_statement::variable_assign_statement(const mcf::ast::expression* name, const mcf::ast::expression* assignExpression) noexcept
	: _name(name)
	, _assignedExpression(assignExpression)
{
	// TODO: #14 assert for _assignExpression == nullptr
}

const std::string mcf::ast::variable_assign_statement::get_name(void) const noexcept
{
	// TODO: #14 assert for _name[i] == nullptr
	const expression* curr = _name.get();
	while (curr->get_expression_type() == expression_type::index)
	{
		curr = static_cast<const index_expression*>(curr)->get_left_expression();
	}

	// TODO: #14 assert for curr->get_expression_type() == expression_type::identifier
	return static_cast<const identifier_expression*>(curr)->convert_to_string();
}

const std::string mcf::ast::variable_assign_statement::convert_to_string(void) const noexcept
{
	std::string buffer;

	buffer += _name->convert_to_string();
	// TODO: #14 assert for _assignExpression == nullptr
	buffer += " = " + _assignedExpression->convert_to_string();
	buffer += ";";

	return buffer;
}

mcf::ast::enum_statement::enum_statement(	const mcf::ast::data_type_expression& name, 
											const mcf::ast::data_type_expression& dataType, 
											const mcf::ast::enum_block_statements_expression* values) noexcept
	: _name(name)
	, _dataType(dataType)
	, _values(values)
{}

const std::string mcf::ast::enum_statement::convert_to_string(void) const noexcept
{
	return "enum " + _name.convert_to_string() + " : " + _dataType.convert_to_string() + " { " + _values->convert_to_string() + " };";
}

const std::string mcf::ast::index_expression::convert_to_string(void) const noexcept
{
	// TODO: #14 assert for _left == nullptr
	return _left->convert_to_string() + "[" + _index->convert_to_string() + "]";
}

const std::string mcf::ast::function_parameter_list_expression::convert_to_string(void) const noexcept
{
	debug_message(u8"#18 기본적인 평가기 개발 구현 필요");
	return std::string();
}

const std::string mcf::ast::function_block_statements_expression::convert_to_string(void) const noexcept
{
	debug_message(u8"#18 기본적인 평가기 개발 구현 필요");
	return std::string();
}

const std::string mcf::ast::function_statement::convert_to_string(void) const noexcept
{
	debug_message(u8"#18 기본적인 평가기 개발 구현 필요");
	return std::string();
}

const std::string mcf::ast::function_parameter_expression::convert_to_string(void) const noexcept
{
	debug_message(u8"#18 기본적인 평가기 개발 구현 필요");
	return std::string();
}

mcf::ast::function_parameter_variadic_expression::function_parameter_variadic_expression(const mcf::token_type& dataFor) noexcept
	: _for(dataFor)
{}

const std::string mcf::ast::function_parameter_variadic_expression::convert_to_string(void) const noexcept
{
	return std::string();
}
