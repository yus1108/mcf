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
