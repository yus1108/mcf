#include "pch.h"
#include "ast.h"
#include "parser.h"
#include "evaluator.h"

mcf::ast::program::program(statement_array&& statements) noexcept
{
	const size_t size = statements.size();
	_statements.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		debug_assert(statements[i].get() != nullptr, u8"statements[%zu]는 nullptr 여선 안됩니다.", i);
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
		debug_assert(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += (i == 0 ? "" : "\n") + _statements[i]->convert_to_string();
	}

	return buffer;
}

void mcf::ast::program::evaluate(mcf::evaluator& inOutEvaluator) const noexcept
{
	const size_t size = _statements.size();
	for (size_t i = 0; i < size; i++)
	{
		debug_assert(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
		_statements[i]->evaluate(inOutEvaluator);
	}
}

mcf::ast::prefix_expression::prefix_expression(const mcf::token& prefix, unique_expression&& targetExpression) noexcept
	: _prefixOperator(prefix)
	, _targetExpression(targetExpression.release()) 
{
	debug_assert(_targetExpression.get() != nullptr, u8"인자로 받은 targetExpression은 nullptr 여선 안됩니다.");
}

const std::string mcf::ast::prefix_expression::convert_to_string(void) const noexcept
{
	debug_assert(_targetExpression.get() != nullptr, u8"_targetExpression은 nullptr 여선 안됩니다.");
	return "(" + _prefixOperator.Literal + _targetExpression->convert_to_string() + ")";
}

mcf::ast::infix_expression::infix_expression(unique_expression&& left, const mcf::token& infix, unique_expression&& right) noexcept
	: _left(left.release())
	, _infixOperator(infix)
	, _right(right.release())
{
	debug_assert(_left.get() != nullptr, u8"인자로 받은 left는 nullptr 여선 안됩니다.");
	debug_assert(_right.get() != nullptr, u8"인자로 받은 right는 nullptr 여선 안됩니다.");
}

const std::string mcf::ast::infix_expression::convert_to_string(void) const noexcept
{
	debug_assert(_left.get() != nullptr, u8"_left는 nullptr 여선 안됩니다.");
	debug_assert(_right.get() != nullptr, u8"_right는 nullptr 여선 안됩니다.");
	return "(" + _left->convert_to_string() + " " + _infixOperator.Literal + " " + _right->convert_to_string() + ")";
}

mcf::ast::index_expression::index_expression(unique_expression&& left, unique_expression&& index) noexcept
	: _left(left.release())
	, _index(index.release()) 
{
	debug_assert(_left.get() != nullptr, u8"인자로 받은 left는 nullptr 여선 안됩니다.");
	debug_assert(_index.get() != nullptr, u8"인자로 받은 index는 nullptr 여선 안됩니다.");
}

const std::string mcf::ast::index_expression::convert_to_string(void) const noexcept
{
	debug_assert(_left.get() != nullptr, u8"_left는 nullptr 여선 안됩니다.");
	debug_assert(_index.get() != nullptr, u8"_index는 nullptr 여선 안됩니다.");
	return _left->convert_to_string() + "[" + _index->convert_to_string() + "]";
}

mcf::ast::function_parameter_expression::function_parameter_expression(const mcf::token& dataFor, unique_expression&& dataType, unique_expression&& name) noexcept
	: _for(dataFor)
	, _type(dataType.release())
	, _name(name.release())
{
	debug_assert(_type.get() != nullptr, u8"인자로 받은 dataType은 nullptr 여선 안됩니다.");
	debug_assert(_name.get() != nullptr, u8"인자로 받은 name은 nullptr 여선 안됩니다.");
}

const std::string mcf::ast::function_parameter_expression::convert_to_string(void) const noexcept
{
	debug_assert(_type.get() != nullptr, u8"_type은 nullptr 여선 안됩니다.");
	debug_assert(_name.get() != nullptr, u8"_name은 nullptr 여선 안됩니다.");
	return _for.Literal + " " + _type->convert_to_string() + " " + _name->convert_to_string();
}

mcf::ast::function_call_expression::function_call_expression(unique_expression&& function, expression_array&& parameters) noexcept
	: _function(function.release())
{
	debug_assert(_function.get() != nullptr, u8"인자로 받은 function은 nullptr 여선 안됩니다.");
	const size_t size = parameters.size();
	_parameters.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		debug_assert(parameters[i].get() != nullptr, u8"parameters[%zu]는 nullptr 여선 안됩니다.", i);
		_parameters.emplace_back(parameters[i].release());
	}
}

const std::string mcf::ast::function_call_expression::convert_to_string(void) const noexcept
{
	debug_assert(_function.get() != nullptr, u8"_function은 nullptr 여선 안됩니다.");

	const size_t size = _parameters.size();
	std::string buffer;

	buffer += _function->convert_to_string();
	buffer += "(";
	for (size_t i = 0; i < size; i++)
	{
		debug_assert(_parameters[i].get() != nullptr, u8"_parameters[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += (i == 0 ? "" : ", ") + _parameters[i]->convert_to_string();
	}
	buffer += ")";
	
	return buffer;
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
		debug_message(u8"variadic은 토큰타입이 invalid거나 in이어야 합니다.");
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
		debug_assert(list[i].get() != nullptr, u8"list[%zu]는 nullptr 여선 안됩니다.", i);
		_list.emplace_back(list[i].release());
	}
}

const std::string mcf::ast::function_parameter_list_expression::convert_to_string(void) const noexcept
{
	std::string buffer;
	const size_t size = _list.size();
	for (size_t i = 0; i < size; i++)
	{
		debug_assert(_list[i].get() != nullptr, u8"_list[%zu]는 nullptr 여선 안됩니다.", i);
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
		debug_assert(statements[i].get() != nullptr, u8"statements[%zu]는 nullptr 여선 안됩니다.", i);
		_statements.emplace_back(statements[i].release());
	}
}

const std::string mcf::ast::function_block_expression::convert_to_string(void) const noexcept
{
	const size_t size = _statements.size();
	std::string buffer;
	for (size_t i = 0; i < size; i++)
	{
		debug_assert(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += "\t" + _statements[i]->convert_to_string() + "\n";
	}
	buffer.erase(buffer.size()  - 1);
	return buffer;
}

mcf::ast::enum_block_expression::enum_block_expression(std::vector<mcf::ast::identifier_expression>& names, expression_array&& values) noexcept
	: _names(names)
{
	const size_t size = values.size();
	debug_assert(names.size() == size, u8"names의 아이템 갯수와 values의 아이템 갯수가 같아야합니다. countof(names)=%zu, countof(values)=%zu", names.size(), size);

	_values.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		debug_assert(values[i].get() != nullptr, u8"values[%zu]는 nullptr 여선 안됩니다.", i);
		_values.emplace_back(values[i].release());
	}
}

const std::string mcf::ast::enum_block_expression::convert_to_string(void) const noexcept
{
	std::string buffer;
	const size_t size = _names.size();

	debug_assert(_values.size() == size, u8"_names의 아이템 갯수와 _values의 아이템 갯수가 같아야합니다. countof(names)=%zu, countof(values)=%zu", size, _values.size());

	for (size_t i = 0; i < size; i++)
	{
		debug_assert(_values[i].get() != nullptr, u8"_values[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += "\t" + _names[i].convert_to_string();
		buffer += _values[i]->get_expression_type() == expression_type::enum_value_increment ? "" : " = ";
		buffer += _values[i]->convert_to_string() + ",\n";
	}
	buffer.erase(buffer.size() - 1);
	return buffer;
}

mcf::ast::macro_include_statement::macro_include_statement(mcf::evaluator* const outEvaluator, std::stack<mcf::parser_error>& outErrors, mcf::token token) noexcept
	: _token(token)
	, _path(token.Literal.substr(sizeof("#include "), token.Literal.size() - sizeof("#include <")))
{
	debug_assert(outEvaluator->include_project_file(_path), u8"파일 include에 실패하였습니다.");
	mcf::parser parser(outEvaluator, _path, true);
	mcf::parser_error parserInitError = parser.get_last_error();
	debug_assert(parserInitError.ID == mcf::parser_error_id::no_error, u8"파일 include에 실패하였습니다. ID=`%s`, File=`%s`(%zu, %zu)\n%s",
		PARSER_ERROR_ID[enum_index(parserInitError.ID)], parserInitError.Name.c_str(), parserInitError.Line, parserInitError.Index, parserInitError.Message.c_str());
	parser.parse_program(_program);
	const size_t errorCount = parser.get_error_count();
	if (errorCount > 0)
	{
		std::stack<mcf::parser_error> errors;
		for ( int i = 0; i < errorCount; ++i )
		{
			errors.push(parser.get_last_error());
		}
		for (int i = 0; i < errorCount; ++i)
		{
			outErrors.push(errors.top());
			errors.pop();
		}
	}
}

void mcf::ast::macro_include_statement::evaluate(mcf::evaluator& inOutEvaluator) const noexcept
{
	inOutEvaluator;
	debug_message("");
}

mcf::ast::variable_statement::variable_statement(const mcf::ast::data_type_expression& dataType, unique_expression&& name, unique_expression&& initExpression) noexcept
	: _dataType(dataType)
	, _name(name.release())
	, _initExpression(initExpression.release())
{
	debug_assert(_name.get() != nullptr, u8"name은 nullptr 여선 안됩니다.");
}

const std::string mcf::ast::variable_statement::get_name(void) const noexcept
{
	debug_assert(_name.get() != nullptr, u8"_name은 nullptr 여선 안됩니다.");
	const expression* curr = _name.get();
	while (curr->get_expression_type() == expression_type::index)
	{
		curr = static_cast<const index_expression*>(curr)->get_left_expression();
	}

	return static_cast<const identifier_expression*>(curr)->convert_to_string();
}

const std::string mcf::ast::variable_statement::convert_to_string(void) const noexcept
{
	debug_assert(_name.get() != nullptr, u8"_name은 nullptr 여선 안됩니다.");

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

void mcf::ast::variable_statement::evaluate(mcf::evaluator& inOutEvaluator) const noexcept
{
	inOutEvaluator;
	debug_message("");
}

mcf::ast::variable_assign_statement::variable_assign_statement(unique_expression&& name, unique_expression&& assignExpression) noexcept
	: _name(name.release())
	, _assignedExpression(assignExpression.release())
{
	debug_assert(_name.get() != nullptr, u8"name은 nullptr 여선 안됩니다.");
	debug_assert(_assignedExpression.get() != nullptr, u8"assignExpression은 nullptr 여선 안됩니다.");
}

const std::string mcf::ast::variable_assign_statement::get_name(void) const noexcept
{
	debug_assert(_name.get() != nullptr, u8"_name은 nullptr 여선 안됩니다.");
	const expression* curr = _name.get();
	while (curr->get_expression_type() == expression_type::index)
	{
		curr = static_cast<const index_expression*>(curr)->get_left_expression();
	}

	return static_cast<const identifier_expression*>(curr)->convert_to_string();
}

const std::string mcf::ast::variable_assign_statement::convert_to_string(void) const noexcept
{
	debug_assert(_name.get() != nullptr, u8"_name은 nullptr 여선 안됩니다.");
	debug_assert(_assignedExpression.get() != nullptr, u8"_assignedExpression은 nullptr 여선 안됩니다.");

	std::string buffer;

	buffer += _name->convert_to_string();
	buffer += " = " + _assignedExpression->convert_to_string();
	buffer += ";";

	return buffer;
}

void mcf::ast::variable_assign_statement::evaluate(mcf::evaluator& inOutEvaluator) const noexcept
{
	inOutEvaluator;
	debug_message("");
}

mcf::ast::function_statement::function_statement(	unique_expression&& returnType, identifier_expression name,
													std::unique_ptr<const mcf::ast::function_parameter_list_expression>&& parameters, 
													std::unique_ptr<const mcf::ast::function_block_expression>&& statementsBlock) noexcept
	: _returnType(returnType.release())
	, _name(name)
	, _parameters(parameters.release())
	, _statementsBlock(statementsBlock.release())
{
	debug_assert(_returnType.get() != nullptr, u8"returnType은 nullptr 여선 안됩니다.");
	debug_assert(_parameters.get() != nullptr, u8"parameters은 nullptr 여선 안됩니다.");
}

const std::string mcf::ast::function_statement::convert_to_string(void) const noexcept
{
	debug_assert(_returnType.get() != nullptr, u8"_returnType은 nullptr 여선 안됩니다.");
	debug_assert(_parameters.get() != nullptr, u8"_parameters은 nullptr 여선 안됩니다.");

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

void mcf::ast::function_statement::evaluate(mcf::evaluator& inOutEvaluator) const noexcept
{
	inOutEvaluator;
	debug_message("");
}

mcf::ast::function_call_statement::function_call_statement(mcf::ast::unique_expression&& callExpression) noexcept
	: _callExpression(callExpression->get_expression_type() == expression_type::function_call ? static_cast<const mcf::ast::function_call_expression*>(callExpression.release()) : nullptr)
{
	debug_assert(_callExpression.get() != nullptr, u8"_callExpression는 nullptr 여선 안됩니다.");
}

mcf::ast::function_call_statement::function_call_statement(std::unique_ptr<mcf::ast::function_call_expression>&& callExpression) noexcept
	: _callExpression(callExpression.release())
{
	debug_assert(_callExpression.get() != nullptr, u8"_callExpression는 nullptr 여선 안됩니다.");
}

mcf::ast::function_call_statement::function_call_statement(std::unique_ptr<const mcf::ast::function_call_expression>&& callExpression) noexcept
	: _callExpression(callExpression.release())
{
	debug_assert(_callExpression.get() != nullptr, u8"_callExpression는 nullptr 여선 안됩니다.");
}

void mcf::ast::function_call_statement::evaluate(mcf::evaluator& inOutEvaluator) const noexcept
{
	inOutEvaluator;
	debug_message("");
}

mcf::ast::enum_statement::enum_statement(	const mcf::ast::data_type_expression& name,
											const mcf::ast::data_type_expression& dataType,
											std::unique_ptr<const mcf::ast::enum_block_expression>&& values) noexcept
	: _name(name)
	, _dataType(dataType)
	, _values(values.release())
{
	debug_assert(_values.get() != nullptr, u8"values는 nullptr 여선 안됩니다.");
}

const std::string mcf::ast::enum_statement::convert_to_string(void) const noexcept
{
	debug_assert(_values.get() != nullptr, u8"_values는 nullptr 여선 안됩니다.");
	return "enum " + _name.convert_to_string() + " : " + _dataType.convert_to_string() + "\n{\n" + _values->convert_to_string() + "\n};";
}

void mcf::ast::enum_statement::evaluate(mcf::evaluator& inOutEvaluator) const noexcept
{
	inOutEvaluator;
	debug_message("");
}