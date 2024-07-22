#include "pch.h"
#include "ast.h"

mcf::ast::program::program(statement_array&& statements) noexcept
{
	const size_t size = statements.size();
	_statements.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		_statements.emplace_back(statements[i].release());
	}
}

const mcf::ast::statement* mcf::ast::program::get_statement_at(const size_t index) const noexcept
{
	return _statements[index].get();
}

const std::string mcf::ast::program::convert_to_string(void) const noexcept
{
	const size_t size = _statements.size();
	std::string buffer;

	for (size_t i = 0; i < size; i++)
	{
		// TODO: #14 assert for _statements[i] == nullptr
		buffer += (i == 0 ? "" : "\n") + _statements[i]->convert_to_string();
	}

	return buffer;
}

const std::vector<mcf::token> mcf::ast::program::convert_to_tokens(void) const noexcept
{
	const size_t size = _statements.size();
	std::vector<token> tokens;

	for (size_t i = 0; i < size; i++)
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

const std::string mcf::ast::infix_expression::convert_to_string(void) const noexcept
{
	return "(" + _left->convert_to_string() + " " + _infixOperator.Literal + " " + _right->convert_to_string() + ")";
}

const std::string mcf::ast::index_expression::convert_to_string(void) const noexcept
{
	// TODO: #14 assert for _left == nullptr
	return _left->convert_to_string() + "[" + _index->convert_to_string() + "]";
}

mcf::ast::function_parameter_expression::function_parameter_expression(const mcf::token& dataFor, const mcf::ast::expression* dataType, const mcf::ast::expression* name) noexcept
	: _for(dataFor)
	, _type(dataType)
	, _name(name)
{}

mcf::ast::function_call_expression::function_call_expression(const mcf::ast::expression* function, expression_array&& parameters) noexcept
	: _function(function)
{
	const size_t size = parameters.size();
	_parameters.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		_parameters.emplace_back(parameters[i].release());
	}
}

const std::string mcf::ast::function_call_expression::convert_to_string(void) const noexcept
{
	const size_t size = _parameters.size();
	std::string buffer;

	buffer += _function->convert_to_string();
	buffer += "(";
	for (size_t i = 0; i < size; i++)
	{
		buffer += (i == 0 ? "" : ", ") + _parameters[i]->convert_to_string();
	}
	buffer += ")";
	
	return buffer;
}

const std::string mcf::ast::function_parameter_expression::convert_to_string(void) const noexcept
{
	return _for.Literal + " " + _type->convert_to_string() + " " + _name->convert_to_string();
}

const std::string mcf::ast::function_parameter_variadic_expression::convert_to_string(void) const noexcept
{
	std::string buffer;
	switch (_for.Type)
	{
	case token_type::keyword_in:
		buffer += "in ";
		break;
	case token_type::invalid:
		break;
	default:
		debug_message("variadic은 토큰타입이 invalid거나 in이어야 합니다.");
		break;
	}
	buffer += "...";
	return buffer;
}

mcf::ast::function_parameter_list_expression::function_parameter_list_expression(expression_array&& list) noexcept
{
	const size_t size = list.size();
	_list.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		_list.emplace_back(list[i].release());
	}
}

const std::string mcf::ast::function_parameter_list_expression::convert_to_string(void) const noexcept
{
	std::string buffer;
	const size_t size = _list.size();
	for (size_t i = 0; i < size; i++)
	{
		buffer += (i == 0 ? "" : ", ") + _list[i]->convert_to_string();
	}
	return buffer;
}

mcf::ast::function_block_expression::function_block_expression(statement_array&& statements) noexcept
{
	const size_t size = statements.size();
	_statements.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		_statements.emplace_back(statements[i].release());
	}
}

const std::string mcf::ast::function_block_expression::convert_to_string(void) const noexcept
{
	const size_t size = _statements.size();
	std::string buffer;
	for (size_t i = 0; i < size; i++)
	{
		buffer += "\t" + _statements[i]->convert_to_string() + "\n";
	}
	buffer.erase(buffer.size()  - 1);
	return buffer;
}

mcf::ast::enum_block_expression::enum_block_expression(std::vector<mcf::ast::identifier_expression >& names, expression_array&& values) noexcept
	: _names(names)
{
	const size_t size = values.size();
	_values.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		_values.emplace_back(values[i].release());
	}
}

const std::string mcf::ast::enum_block_expression::convert_to_string(void) const noexcept
{
	std::string buffer;

	const size_t size = _names.size();
	for (size_t i = 0; i < size; i++)
	{
		// TODO: #14 assert for _assignExpression == nullptr
		buffer += "\t" + _names[i].convert_to_string();
		buffer += _values[i]->get_expression_type() == expression_type::enum_value_increment ? "" : " = ";
		buffer += _values[i]->convert_to_string() + ",\n";
	}
	buffer.erase(buffer.size() - 1);
	return buffer;
}

mcf::ast::macro_include_statement::macro_include_statement(mcf::token token) noexcept
	: _token(token)
	, _path(token.Literal.substr(sizeof("#include <")))
{}

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

mcf::ast::function_statement::function_statement(	const mcf::ast::expression* returnType, identifier_expression name, 
													const mcf::ast::function_parameter_list_expression* parameters, 
													const mcf::ast::function_block_expression* statementsBlock) noexcept
	: _returnType(returnType)
	, _name(name)
	, _parameters(parameters)
	, _statementsBlock(statementsBlock)
{}

const std::string mcf::ast::function_statement::convert_to_string(void) const noexcept
{
	std::string buffer;

	buffer = _returnType->convert_to_string();
	buffer += " " + _name.convert_to_string();
	buffer += "(" + _parameters->convert_to_string() + ")";
	if (_statementsBlock.get() == nullptr)
	{
		buffer += ";";
	}
	else
	{
		buffer += "\n{\n" + _statementsBlock->convert_to_string() + "\n}";
	}

	return buffer;
}

mcf::ast::enum_statement::enum_statement(const mcf::ast::data_type_expression& name,
	const mcf::ast::data_type_expression& dataType,
	const mcf::ast::enum_block_expression* values) noexcept
	: _name(name)
	, _dataType(dataType)
	, _values(values)
{}

const std::string mcf::ast::enum_statement::convert_to_string(void) const noexcept
{
	return "enum " + _name.convert_to_string() + " : " + _dataType.convert_to_string() + "\n{\n" + _values->convert_to_string() + "\n};";
}