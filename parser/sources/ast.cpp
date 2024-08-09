#include "pch.h"
#include "parser/includes/ast.h"

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

mcf::AST::Intermediate::MapInitializer::MapInitializer(std::vector<KeyValue>&& itemList) noexcept
	: _itemList(std::move(itemList))
{
#if defined(_DEBUG)
	const size_t size = _itemList.size();
	DebugAssert(size != 0, u8"_itemList에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_itemList[i].first.get() != nullptr, u8"_itemList[%zu].key는 nullptr 여선 안됩니다.", i);
		DebugAssert(_itemList[i].second.get() != nullptr, u8"_itemList[%zu].value는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::AST::Intermediate::MapInitializer::ConvertToString(void) const noexcept
{
	const size_t itemListCount = _itemList.size();
	DebugAssert(itemListCount != 0, u8"_itemList에 값이 최소 한개 이상 있어야 합니다.");

	std::string buffer;
	buffer = "<MapInitializer: LBRACE ";
	for (size_t i = 0; i < itemListCount; i++)
	{
		DebugAssert(_itemList[i].first.get() != nullptr, u8"_itemList[%zu].key는 nullptr 여선 안됩니다.", i);
		DebugAssert(_itemList[i].second.get() != nullptr, u8"_itemList[%zu].value는 nullptr 여선 안됩니다.", i);
		buffer += _itemList[i].first->ConvertToString() + " ASSIGN " + _itemList[i].second->ConvertToString() + " COMMA ";
	}
	buffer += "RBRACE>";
	return buffer;
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
