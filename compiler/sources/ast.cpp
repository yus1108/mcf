﻿#include "pch.h"
#include "ast.h"

mcf::AST::Expression::Prefix::Prefix(const mcf::Token::Data& prefixOperator, mcf::AST::Expression::Pointer&& right) noexcept
	: _prefixOperator(prefixOperator)
	, _right(std::move(right))
{
	MCF_DEBUG_ASSERT(_right.get() != nullptr, u8"인자로 받은 _right는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Expression::Prefix::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_right.get() != nullptr, u8"인자로 받은 _right는 nullptr 여선 안됩니다.");
	return std::string("<Prefix: ") + mcf::Token::CONVERT_TYPE_TO_STRING(_prefixOperator.Type) + " " + _right->ConvertToString() + ">";
}

mcf::AST::Expression::Group::Group(mcf::AST::Expression::Pointer && expression) noexcept
	: _expression(std::move(expression))
{
	MCF_DEBUG_ASSERT( _expression.get() != nullptr, u8"인자로 받은 _expression는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Expression::Group::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT( _expression.get() != nullptr, u8"인자로 받은 _expression는 nullptr 여선 안됩니다." );
	return "<Group: " + _expression->ConvertToString() + ">";
}

mcf::AST::Expression::Infix::Infix(mcf::AST::Expression::Pointer&& left, const mcf::Token::Data& infixOperator, mcf::AST::Expression::Pointer&& right) noexcept
	: _infixOperator(infixOperator)
	, _left(std::move(left))
	, _right(std::move(right))
{
	MCF_DEBUG_ASSERT(_infixOperator.Type != mcf::Token::Type::INVALID, u8"인자로 받은 _infixOperator의 타입은 INVALID여선 안됩니다.");
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_right.get() != nullptr, u8"인자로 받은 _right는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Expression::Infix::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_right.get() != nullptr, u8"인자로 받은 _right는 nullptr 여선 안됩니다.");
	return "<Infix: " + _left->ConvertToString() + " " + mcf::Token::CONVERT_TYPE_TO_STRING(_infixOperator.Type) + " " + _right->ConvertToString() + ">";
}

mcf::AST::Expression::Call::Call(mcf::AST::Expression::Pointer&& left, mcf::AST::Expression::PointerVector&& params) noexcept
	: _left(std::move(left))
	, _params(std::move(params))
{
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
#if defined(_DEBUG)
	const size_t paramsCount = _params.size();
	for (size_t i = 0; i < paramsCount; i++)
	{
		MCF_DEBUG_ASSERT(_params[i].get() != nullptr, u8"_keyList[%zu].key는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::AST::Expression::Call::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
	std::string buffer = "<Call: " + _left->ConvertToString() + " LPAREN ";
	const size_t paramsCount = _params.size();
	for (size_t i = 0; i < paramsCount; i++)
	{
		MCF_DEBUG_ASSERT(_params[i].get() != nullptr, u8"_params[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += _params[i]->ConvertToString() + " COMMA ";
	}
	return buffer + "RPAREN>";
}

mcf::AST::Expression::Index::Index(mcf::AST::Expression::Pointer&& left, mcf::AST::Expression::Pointer&& index) noexcept
	: _left(std::move(left))
	, _index(std::move(index))
{
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Expression::Index::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
	return (_index.get() == nullptr) ?
		("<Index: " + _left->ConvertToString() + " LBRACKET RBRACKET>") : 
		("<Index: " + _left->ConvertToString() + " LBRACKET " + _index->ConvertToString() + " RBRACKET>");
}

mcf::AST::Expression::As::As(mcf::AST::Expression::Pointer&& left, mcf::AST::Intermediate::TypeSignature::Pointer&& typeSignature) noexcept
	: _left(std::move(left))
	, _typeSignature(std::move(typeSignature))
{
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_typeSignature.get() != nullptr, u8"인자로 받은 _typeSignature는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Expression::As::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_typeSignature.get() != nullptr, u8"인자로 받은 _typeSignature는 nullptr 여선 안됩니다.");
	return "<As: " + _left->ConvertToString() + " KEYWORD_AS " + _typeSignature->ConvertToString() + ">";
}

mcf::AST::Expression::Initializer::Initializer(PointerVector&& keyList) noexcept
	: _keyList(std::move(keyList))
{
#if defined(_DEBUG)
	const size_t size = _keyList.size();
	MCF_DEBUG_ASSERT(size != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_keyList[i].get() != nullptr, u8"_keyList[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::AST::Expression::Initializer::ConvertToString(void) const noexcept
{
	const size_t keyListCount = _keyList.size();
	MCF_DEBUG_ASSERT(keyListCount != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");

	std::string buffer;
	buffer = "<Initializer: LBRACE ";
	for (size_t i = 0; i < keyListCount; i++)
	{
		MCF_DEBUG_ASSERT(_keyList[i].get() != nullptr, u8"_keyList[%zu].key는 nullptr 여선 안됩니다.", i);
		buffer += _keyList[i]->ConvertToString() + " COMMA ";
	}
	buffer += "RBRACE>";
	return buffer;
}

mcf::AST::Expression::MapInitializer::MapInitializer(PointerVector&& keyist, PointerVector&& valueList) noexcept
	: Initializer(std::move(keyist))
	, _valueList(std::move(valueList))
{
#if defined(_DEBUG)
	const size_t size = _valueList.size();
	MCF_DEBUG_ASSERT(size != 0, u8"_valueList에 값이 최소 한개 이상 있어야 합니다.");
	MCF_DEBUG_ASSERT(_keyList.size() == size, u8"_valueList의 갯수와 _keyList의 갯수가 동일 해야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_valueList[i].get() != nullptr, u8"_valueList[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::AST::Expression::MapInitializer::ConvertToString(void) const noexcept
{
	const size_t keyListCount = _keyList.size();
	MCF_DEBUG_ASSERT(keyListCount != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");
	MCF_DEBUG_ASSERT(_valueList.size() == keyListCount, u8"_valueList의 갯수와 _keyList의 갯수가 동일 해야 합니다.");

	std::string buffer;
	buffer = "<MapInitializer: LBRACE ";
	for (size_t i = 0; i < keyListCount; i++)
	{
		MCF_DEBUG_ASSERT(_keyList[i].get() != nullptr, u8"_keyList[%zu].key는 nullptr 여선 안됩니다.", i);
		MCF_DEBUG_ASSERT(_valueList[i].get() != nullptr, u8"_valueList[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += _keyList[i]->ConvertToString() + " ASSIGN " + _valueList[i]->ConvertToString() + " COMMA ";
	}
	buffer += "RBRACE>";
	return buffer;
}

mcf::AST::Intermediate::Variadic::Variadic(mcf::AST::Expression::Identifier::Pointer&& name) noexcept
	: _name(std::move(name))
{
	MCF_DEBUG_ASSERT(_name.get() != nullptr, u8"인자로 받은 _name는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Intermediate::Variadic::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_name.get() != nullptr, u8"인자로 받은 _name는 nullptr 여선 안됩니다.");
	return "<Variadic: " + _name->ConvertToString() + ">";
}

mcf::AST::Intermediate::TypeSignature::TypeSignature(const bool isUnsigned, mcf::AST::Expression::Pointer&& signature) noexcept
	: _isUnsigned(isUnsigned)
	, _signature(std::move(signature))
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"_signature는 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT
	(
		_signature->GetExpressionType() == mcf::AST::Expression::Type::IDENTIFIER || _signature->GetExpressionType() == mcf::AST::Expression::Type::INDEX, 
		u8"_signature는 identifier 또는 index expression 이어야 합니다."
	);
}

const mcf::AST::Expression::Interface* mcf::AST::Intermediate::TypeSignature::GetUnsafeSignaturePointer(void) const noexcept
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"_signature는 nullptr 여선 안됩니다.");
	return _signature.get();
}

const std::string mcf::AST::Intermediate::TypeSignature::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"_signature는 nullptr 여선 안됩니다.");
	return "<TypeSignature: " + std::string(_isUnsigned ? "KEYWORD_UNSIGNED " : "") + _signature->ConvertToString() + ">";
}

mcf::AST::Intermediate::VariableSignature::VariableSignature(mcf::AST::Expression::Identifier::Pointer&& name, TypeSignature::Pointer&& typeSignature) noexcept
	: _name(std::move(name))
	, _typeSignature(std::move(typeSignature))
{
	MCF_DEBUG_ASSERT(_name.get() != nullptr, u8"인자로 받은 _name은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_typeSignature.get() != nullptr, u8"인자로 받은 _typeSignature는 nullptr 여선 안됩니다.");
}

const mcf::AST::Intermediate::TypeSignature* mcf::AST::Intermediate::VariableSignature::GetUnsafeTypeSignaturePointer(void) const noexcept
{
	MCF_DEBUG_ASSERT(_typeSignature.get() != nullptr, u8"인자로 받은 _typeSignature는 nullptr 여선 안됩니다.");
	return _typeSignature.get();
}

const std::string mcf::AST::Intermediate::VariableSignature::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_name.get() != nullptr, u8"인자로 받은 _name은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_typeSignature.get() != nullptr, u8"인자로 받은 _typeSignature는 nullptr 여선 안됩니다.");
	return "<VariableSignature: " + _name->ConvertToString() + " COLON " + _typeSignature->ConvertToString() + ">";
}

const mcf::AST::Intermediate::VariableSignature* mcf::AST::Intermediate::FunctionParams::GetUnsafeParamPointerAt(size_t index) const noexcept
{
	MCF_DEBUG_ASSERT(index < _params.size(), u8"인자로 받은 index는 _params의 사이즈보다 작아야 합니다.");
	MCF_DEBUG_ASSERT(_params[index].get() != nullptr, u8"인자로 받은 _params[%zu]는 nullptr 여선 안됩니다.", index);
	return _params[index].get();
}

const mcf::AST::Intermediate::Variadic* mcf::AST::Intermediate::FunctionParams::GetUnsafeVariadic(void) const noexcept
{
	MCF_DEBUG_ASSERT( _variadic.get() != nullptr, u8"인자로 받은 _variadic은 nullptr 여선 안됩니다.");
	return _variadic.get();
}

const std::string mcf::AST::Intermediate::FunctionParams::ConvertToString(void) const noexcept
{
	if (IsVoid())
	{
		return "<FunctionParams: LPAREN KEYWORD_VOID RPAREN>";
	}

	std::string buffer = "<FunctionParams: LPAREN ";
	if (HasParams() == true)
	{
		const size_t paramsCount = _params.size();
		for (size_t i = 0; i < paramsCount; i++)
		{
			buffer += _params[i]->ConvertToString() + " COMMA ";
		}
	}
	if (HasVariadic() == true)
	{
		buffer += _variadic->ConvertToString() + " ";
	}
	buffer += "RPAREN>";
	return buffer;
}

mcf::AST::Intermediate::FunctionSignature::FunctionSignature(mcf::AST::Expression::Identifier::Pointer name, FunctionParams::Pointer params, TypeSignature::Pointer returnType) noexcept
	: _name(std::move(name))
	, _params(std::move(params))
	, _returnType(std::move(returnType))
{
	MCF_DEBUG_ASSERT(_name.get() != nullptr, u8"인자로 받은 _name은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_params.get() != nullptr, u8"인자로 받은 _params은 nullptr 여선 안됩니다.");
}

const mcf::AST::Intermediate::TypeSignature* mcf::AST::Intermediate::FunctionSignature::GetUnsafeReturnTypePointer(void) const noexcept
{
	MCF_DEBUG_ASSERT(_returnType.get() != nullptr, u8"리턴 타입이 void일 경우 리턴 타입을 가져올 수 없습비다.");
	return _returnType.get();
}

const mcf::AST::Intermediate::FunctionParams* mcf::AST::Intermediate::FunctionSignature::GetUnsafeFunctionParamsPointer(void) const noexcept
{
	MCF_DEBUG_ASSERT(_params.get() != nullptr, u8"인자로 받은 _params은 nullptr 여선 안됩니다.");
	return _params.get();
}

const std::string mcf::AST::Intermediate::FunctionSignature::ConvertToString(void) const noexcept
{
	std::string buffer = "<FunctionSignature: " + _name->ConvertToString() + " " + _params->ConvertToString() + " POINTING ";
	buffer += IsReturnTypeVoid() ? "KEYWORD_VOID" : _returnType->ConvertToString();
	return buffer + ">";
}


const std::string mcf::AST::Statement::IncludeLibrary::ConvertToString(void) const noexcept
{
	std::string buffer;
	buffer = "[IncludeLibrary: LT ";
	buffer += "KEYWORD_ASM COMMA " + _libPath.Literal;
	buffer += " GT]";
	return buffer;
}

mcf::AST::Statement::Typedef::Typedef(SignaturePointer&& signature) noexcept
	: _signature(std::move(signature))
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::Typedef::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
	return "[Typedef: " + _signature->ConvertToString() + " SEMICOLON]";
}

mcf::AST::Statement::Extern::Extern(mcf::AST::Intermediate::FunctionSignature::Pointer&& signature) noexcept
	: _signature(std::move(signature))
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::Extern::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
	return "[Extern " + _signature->ConvertToString() + " SEMICOLON]";
}

mcf::AST::Statement::Let::Let(mcf::AST::Intermediate::VariableSignature::Pointer&& signature, mcf::AST::Expression::Pointer&& expression) noexcept
	: _signature(std::move(signature))
	, _expression(std::move(expression))
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::Let::ConvertToString(void) const noexcept
{
	return "[Let: " + _signature->ConvertToString() + (_expression.get() == nullptr ? "" : (" ASSIGN " + _expression->ConvertToString())) + " SEMICOLON]";
}

mcf::AST::Statement::Block::Block(Statement::PointerVector&& statements) noexcept
	: _statements(std::move(statements))
{
#if defined(_DEBUG)
		const size_t size = _statements.size();
		for (size_t i = 0; i < size; i++)
		{
			MCF_DEBUG_ASSERT(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
		}
#endif
}

const std::string mcf::AST::Statement::Block::ConvertToString(void) const noexcept
{
	const size_t size = _statements.size();
	std::string buffer = "[Block: LBRACE ";

	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += _statements[i]->ConvertToString() + " ";
	}

	return buffer + "RBRACE]";
}

mcf::AST::Statement::Return::Return(mcf::AST::Expression::Pointer&& returnValue) noexcept
	: _returnValue(std::move(returnValue))
{
	MCF_DEBUG_ASSERT(_returnValue.get() != nullptr, u8"인자로 받은 _returnValue은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::Return::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_returnValue.get() != nullptr, u8"인자로 받은 _returnValue은 nullptr 여선 안됩니다.");
	return "[Return: " + _returnValue->ConvertToString() + " SEMICOLON]";
}

mcf::AST::Statement::Func::Func(mcf::AST::Intermediate::FunctionSignature::Pointer&& signature, mcf::AST::Statement::Block::Pointer&& block) noexcept
	: _signature(std::move(signature))
	, _block(std::move(block))
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_block.get() != nullptr, u8"인자로 받은 _block은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::Func::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_block.get() != nullptr, u8"인자로 받은 _block은 nullptr 여선 안됩니다.");
	return "[Func: " + _signature->ConvertToString() + " " + _block->ConvertToString() + "]";
}

std::string mcf::AST::Statement::Main::NAME = std::string("main");

mcf::AST::Statement::Main::Main(mcf::AST::Intermediate::FunctionParams::Pointer&& params, mcf::AST::Intermediate::TypeSignature::Pointer&& returnType, Block::Pointer&& block) noexcept
	: _params(std::move(params))
	, _returnType(std::move(returnType))
	, _block(std::move(block))
{
	MCF_DEBUG_ASSERT(_params.get() != nullptr, u8"인자로 받은 _params은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_block.get() != nullptr, u8"인자로 받은 _block은 nullptr 여선 안됩니다.");
}

const mcf::AST::Intermediate::TypeSignature* mcf::AST::Statement::Main::GetUnsafeReturnTypePointer(void) const noexcept
{
	MCF_DEBUG_ASSERT(_returnType.get() != nullptr, u8"리턴 타입이 void일 경우 리턴 타입을 가져올 수 없습비다.");
	return _returnType.get();
}

const mcf::AST::Intermediate::FunctionParams* mcf::AST::Statement::Main::GetUnsafeFunctionParamsPointer(void) const noexcept
{
	MCF_DEBUG_ASSERT(_params.get() != nullptr, u8"인자로 받은 _params은 nullptr 여선 안됩니다.");
	return _params.get();
}

const mcf::AST::Statement::Block* mcf::AST::Statement::Main::GetUnsafeBlockPointer(void) const noexcept
{
	MCF_DEBUG_ASSERT(_block.get() != nullptr, u8"인자로 받은 _block은 nullptr 여선 안됩니다.");
	return _block.get();
}

const std::string mcf::AST::Statement::Main::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_params.get() != nullptr, u8"인자로 받은 _params은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_block.get() != nullptr, u8"인자로 받은 _block은 nullptr 여선 안됩니다.");

	if (IsReturnVoid())
	{
		return "[Main: " + _params->ConvertToString() + " POINTING KEYWORD_VOID " + _block->ConvertToString() + "]";
	}
	return "[Main: " + _params->ConvertToString() + " POINTING " + _returnType->ConvertToString() + " " + _block->ConvertToString() + "]";
}

mcf::AST::Statement::Expression::Expression(mcf::AST::Expression::Pointer&& expression) noexcept
	: _expression(std::move(expression))
{
	MCF_DEBUG_ASSERT(_expression.get() != nullptr, u8"인자로 받은 _expression은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::Expression::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_expression.get() != nullptr, u8"인자로 받은 _expression은 nullptr 여선 안됩니다.");
	return "[Expression: " + _expression->ConvertToString() + " SEMICOLON]";
}

mcf::AST::Statement::AssignExpression::AssignExpression(mcf::AST::Expression::Pointer&& left, mcf::AST::Expression::Pointer&& right) noexcept
	: _left(std::move(left))
	, _right(std::move(right))
{
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_right.get() != nullptr, u8"인자로 받은 _right은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::AssignExpression::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_left.get() != nullptr, u8"인자로 받은 _left은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_right.get() != nullptr, u8"인자로 받은 _right은 nullptr 여선 안됩니다.");
	return "[Expression: " + _left->ConvertToString() + " ASSIGN " + _right->ConvertToString() + " SEMICOLON]";
}

mcf::AST::Statement::Unused::Unused(mcf::AST::Expression::Identifier::PointerVector&& identifiers) noexcept
	: _identifiers(std::move(identifiers))
{
#if defined(_DEBUG)
	const size_t size = _identifiers.size();
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_identifiers[i].get() != nullptr, u8"_identifiers[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::AST::Statement::Unused::ConvertToString(void) const noexcept
{
	std::string buffer = "[Unused: LPAREN ";
	const size_t size = _identifiers.size();
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_identifiers[i].get() != nullptr, u8"_identifiers[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += _identifiers[i]->ConvertToString() + " COMMA ";
	}
	return buffer + "RPAREN SEMICOLON]";
}

mcf::AST::Statement::While::While(mcf::AST::Expression::Pointer&& condition, mcf::AST::Statement::Block::Pointer&& block) noexcept
	: _condition(std::move(condition))
	, _block(std::move(block))
{
	MCF_DEBUG_ASSERT(_condition.get() != nullptr, u8"인자로 받은 _condition은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_block.get() != nullptr, u8"인자로 받은 _block은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::While::ConvertToString(void) const noexcept
{
	MCF_DEBUG_ASSERT(_condition.get() != nullptr, u8"인자로 받은 _condition은 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_block.get() != nullptr, u8"인자로 받은 _block은 nullptr 여선 안됩니다.");
	return "[While: LPAREN " + _condition->ConvertToString() + " RPAREN " + _block->ConvertToString() + "]";
}

mcf::AST::Program::Program(mcf::AST::Statement::PointerVector&& statements) noexcept
	: _statements(std::move(statements))
{
#if defined(_DEBUG)
	const size_t size = _statements.size();
	MCF_DEBUG_ASSERT(size != 0, u8"_statements에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::AST::Program::ConvertToString(void) const noexcept
{
	const size_t size = _statements.size();
	MCF_DEBUG_ASSERT(size != 0, u8"_statements에 값이 최소 한개 이상 있어야 합니다.");
	std::string buffer;

	for (size_t i = 0; i < size; i++)
	{
		MCF_DEBUG_ASSERT(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += (i == 0 ? "" : "\n") + _statements[i]->ConvertToString();
	}

	return buffer;
}