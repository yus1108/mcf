#include "pch.h"
#include "parser/includes/ast.h"

mcf::AST::Expression::Infix::Infix(mcf::AST::Expression::Pointer&& left, const mcf::Token::Data& infixOperator, mcf::AST::Expression::Pointer&& right) noexcept
	: _infixOperator(infixOperator)
	, _left(left.release())
	, _right(right.release())
{
	DebugAssert(_infixOperator.Type != mcf::Token::Type::INVALID, u8"인자로 받은 _infixOperator의 타입은 INVALID여선 안됩니다.");
	DebugAssert(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
	DebugAssert(_right.get() != nullptr, u8"인자로 받은 _right는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Expression::Infix::ConvertToString(void) const noexcept
{
	DebugAssert(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
	DebugAssert(_right.get() != nullptr, u8"인자로 받은 _right는 nullptr 여선 안됩니다.");
	return "<Infix: " + _left->ConvertToString() + " " + mcf::Token::CONVERT_TYPE_TO_STRING(_infixOperator.Type) + " " + _right->ConvertToString() + ">";
}

mcf::AST::Expression::Index::Index(mcf::AST::Expression::Pointer&& left, mcf::AST::Expression::Pointer&& index) noexcept
	: _left(left.release())
	, _index(index.release())
{
	DebugAssert(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Expression::Index::ConvertToString(void) const noexcept
{
	DebugAssert(_left.get() != nullptr, u8"인자로 받은 _left는 nullptr 여선 안됩니다.");
	return (_index.get() == nullptr) ?
		("<Index: " + _left->ConvertToString() + " LBRACKET RBRACKET>") : 
		("<Index: " + _left->ConvertToString() + " LBRACKET " + _index->ConvertToString() + " RBRACKET>");
}

mcf::AST::Expression::Initializer::Initializer(PointerVector&& keyList) noexcept
	: _keyList(std::move(keyList))
{
#if defined(_DEBUG)
	const size_t size = _keyList.size();
	DebugAssert(size != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_keyList[i].get() != nullptr, u8"_keyList[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::AST::Expression::Initializer::ConvertToString(void) const noexcept
{
	const size_t keyListCount = _keyList.size();
	DebugAssert(keyListCount != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");

	std::string buffer;
	buffer = "<Initializer: LBRACE ";
	for (size_t i = 0; i < keyListCount; i++)
	{
		DebugAssert(_keyList[i].get() != nullptr, u8"_keyList[%zu].key는 nullptr 여선 안됩니다.", i);
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
	DebugAssert(size != 0, u8"_valueList에 값이 최소 한개 이상 있어야 합니다.");
	DebugAssert(_keyList.size() == size, u8"_valueList의 갯수와 _keyList의 갯수가 동일 해야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_valueList[i].get() != nullptr, u8"_valueList[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::AST::Expression::MapInitializer::ConvertToString(void) const noexcept
{
	const size_t keyListCount = _keyList.size();
	DebugAssert(keyListCount != 0, u8"_keyList에 값이 최소 한개 이상 있어야 합니다.");
	DebugAssert(_valueList.size() == keyListCount, u8"_valueList의 갯수와 _keyList의 갯수가 동일 해야 합니다.");

	std::string buffer;
	buffer = "<MapInitializer: LBRACE ";
	for (size_t i = 0; i < keyListCount; i++)
	{
		DebugAssert(_keyList[i].get() != nullptr, u8"_keyList[%zu].key는 nullptr 여선 안됩니다.", i);
		DebugAssert(_valueList[i].get() != nullptr, u8"_valueList[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += _keyList[i]->ConvertToString() + " ASSIGN " + _valueList[i]->ConvertToString() + " COMMA ";
	}
	buffer += "RBRACE>";
	return buffer;
}

mcf::AST::Intermediate::Variadic::Variadic(mcf::AST::Expression::Identifier::Pointer&& name) noexcept
	: _name(name.release())
{
	DebugAssert(_name.get() != nullptr, u8"인자로 받은 _name는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Intermediate::Variadic::ConvertToString(void) const noexcept
{
	DebugAssert(_name.get() != nullptr, u8"인자로 받은 _name는 nullptr 여선 안됩니다.");
	return "<Variadic: " + _name->ConvertToString() + ">";
}

mcf::AST::Intermediate::TypeSignature::TypeSignature(mcf::AST::Expression::Pointer&& signature) noexcept
	: _signature(signature.release())
{
	DebugAssert(_signature.get() != nullptr, u8"_signature는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Intermediate::TypeSignature::ConvertToString(void) const noexcept
{
	DebugAssert(_signature.get() != nullptr, u8"_signature는 nullptr 여선 안됩니다.");
	return "<TypeSignature: " + _signature->ConvertToString() + ">";
}

mcf::AST::Intermediate::VariableSignature::VariableSignature(mcf::AST::Expression::Identifier::Pointer&& name, TypeSignature::Pointer&& typeSignature) noexcept
	: _name(name.release())
	, _typeSignature(typeSignature.release())
{
	DebugAssert(_name.get() != nullptr, u8"인자로 받은 _name은 nullptr 여선 안됩니다.");
	DebugAssert(_typeSignature.get() != nullptr, u8"인자로 받은 _typeSignature는 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Intermediate::VariableSignature::ConvertToString(void) const noexcept
{
	return "<VariableSignature: " + _name->ConvertToString() + " COLON " + _typeSignature->ConvertToString() + ">";
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
	DebugAssert(_name.get() != nullptr, u8"인자로 받은 _name은 nullptr 여선 안됩니다.");
	DebugAssert(_params.get() != nullptr, u8"인자로 받은 _params은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Intermediate::FunctionSignature::ConvertToString(void) const noexcept
{
	std::string buffer = "<FunctionSignature: " + _name->ConvertToString() + " " + _params->ConvertToString() + " POINTING ";
	buffer += IsReturnVoid() ? "KEYWORD_VOID" : _returnType->ConvertToString();
	return buffer + ">";
}


const std::string mcf::AST::Statement::IncludeLibrary::ConvertToString(void) const noexcept
{
	std::string buffer;
	buffer = "[IncludeLibrary: LT ";
	buffer += "asm COMMA " + _libPath.Literal;
	buffer += " GT]";
	return buffer;
}

mcf::AST::Statement::Typedef::Typedef(SignaturePointer&& signature, BindMapPointer&& bindMap) noexcept
	: _signature(std::move(signature))
	, _bindMap(std::move(bindMap))
{
	DebugAssert(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::Typedef::ConvertToString(void) const noexcept
{
	DebugAssert(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");

	std::string buffer = "[Typedef: " + _signature->ConvertToString();
	if (_bindMap.get() != nullptr)
	{
		buffer += " POINTING KEYWORD_BIND " + _bindMap->ConvertToString();;
	}
	buffer += " SEMICOLON]";
	return buffer;
}

mcf::AST::Statement::Extern::Extern(const bool isAssemblyFunction, mcf::AST::Intermediate::FunctionSignature::Pointer&& signature) noexcept
	: _isAssemblyFunction(isAssemblyFunction)
	, _signature(std::move(signature))
{
	DebugAssert(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
}

const std::string mcf::AST::Statement::Extern::ConvertToString(void) const noexcept
{
	DebugAssert(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
	return "[Extern" + std::string(_isAssemblyFunction ? " KEYWORD_ASM " : " ") + _signature->ConvertToString() + " SEMICOLON]";
}

mcf::AST::Statement::Let::Let(mcf::AST::Intermediate::VariableSignature::Pointer&& signature, mcf::AST::Expression::Pointer&& expression) noexcept
	: _signature(std::move(signature))
	, _expression(std::move(expression))
{
	DebugAssert(_signature.get() != nullptr, u8"인자로 받은 _signature은 nullptr 여선 안됩니다.");
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
		DebugAssert(size != 0, u8"_statements에 값이 최소 한개 이상 있어야 합니다.");
		for (size_t i = 0; i < size; i++)
		{
			DebugAssert(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
		}
#endif
}

const std::string mcf::AST::Statement::Block::ConvertToString(void) const noexcept
{
	const size_t size = _statements.size();
	DebugAssert(size != 0, u8"_statements에 값이 최소 한개 이상 있어야 합니다.");
	std::string buffer = "[Block: LBRACE ";

	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += _statements[i]->ConvertToString() + " ";
	}

	return buffer + "RBRACE]";
}

mcf::AST::Program::Program(mcf::AST::Statement::PointerVector&& statements) noexcept
	: _statements(std::move(statements))
{
#if defined(_DEBUG)
	const size_t size = _statements.size();
	DebugAssert(size != 0, u8"_statements에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::AST::Program::ConvertToString(void) const noexcept
{
	const size_t size = _statements.size();
	DebugAssert(size != 0, u8"_statements에 값이 최소 한개 이상 있어야 합니다.");
	std::string buffer;

	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_statements[i].get() != nullptr, u8"_statements[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += (i == 0 ? "" : "\n") + _statements[i]->ConvertToString();
	}

	return buffer;
}