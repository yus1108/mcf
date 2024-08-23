#include "pch.h"
#include "evaluator.h"
#include "object.h"

const std::string mcf::Object::TypeInfo::Inspect( void ) const noexcept
{
	DebugAssert(IsValid(), u8"TypeInfo가 유효하지 않습니다.");
	std::string buffer = std::string(IsUnsigned ? "unsigned " : "") + Name;
	const size_t arraySize = ArraySizeList.size();
	for (size_t i = 0; i < arraySize; ++i)
	{
		DebugAssert(ArraySizeList[i] > 0, u8"ArraySizeList[%zu]는 0이상 이어야 합니다. 값=%zu", i, ArraySizeList[i]);
		buffer += "[" + std::to_string(ArraySizeList[i]) + "]";
	}
	return buffer;
}

const bool mcf::Object::Scope::IsIdentifierRegistered(const std::string& name) const noexcept
{
	if (_allIdentifierSet.find( name ) != _allIdentifierSet.end())
	{
		return true;
	}
	if (_parent == nullptr)
	{
		return false;
	}
	return _parent->IsIdentifierRegistered(name);
}

const bool mcf::Object::Scope::DefineType(const std::string& name, const mcf::Object::TypeInfo& info) noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	DebugAssert(info.IsValid(), u8"함수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		DebugMessage(u8"구현 필요");
		return false;
	}

	_allIdentifierSet.emplace(name);
	_typeInfoMap.emplace(name, info);

	return true;
}

const mcf::Object::TypeInfo mcf::Object::Scope::FindTypeInfo(const std::string& name) const noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _typeInfoMap.find(name);
	if (infoFound == _typeInfoMap.end())
	{
		if (_parent == nullptr)
		{
			return mcf::Object::TypeInfo();
		}
		return _parent->FindTypeInfo(name);
	}
	return infoFound->second;
}

const bool mcf::Object::Scope::DefineVariable(const std::string& name, const mcf::Object::Variable& info) noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	DebugAssert(info.IsValid(), u8"변수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		DebugMessage(u8"구현 필요");
		return false;
	}

	_allIdentifierSet.emplace(name);
	_variables.emplace(name, info);

	return true;
}

const mcf::Object::VariableInfo mcf::Object::Scope::FindVariableInfo(const std::string& name) const noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _variables.find(name);
	if (infoFound == _variables.end())
	{
		if (_parent == nullptr)
		{
			return mcf::Object::VariableInfo();
		}
		return _parent->FindVariableInfo(name);
	}
	return { infoFound->second, _parent == nullptr };
}

const bool mcf::Object::Scope::DefineFunction(const std::string& name, const mcf::Object::FunctionInfo& info) noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");
	DebugAssert(info.IsValid(), u8"함수 정보가 유효하지 않습니다.");

	if (_allIdentifierSet.find(name) != _allIdentifierSet.end())
	{
		DebugMessage(u8"구현 필요");
		return false;
	}

	_allIdentifierSet.emplace(name);
	_functionInfoMap.emplace(name, info);

	return true;
}

const mcf::Object::FunctionInfo mcf::Object::Scope::FindFunction(const std::string& name) const noexcept
{
	DebugAssert(name.empty() == false, u8"이름이 비어 있으면 안됩니다.");

	auto infoFound = _functionInfoMap.find(name);
	if (infoFound == _functionInfoMap.end())
	{
		if (_parent == nullptr)
		{
			return mcf::Object::FunctionInfo();
		}
		return _parent->FindFunction(name);
	}
	return infoFound->second;
}

mcf::IR::IncludeLib::IncludeLib(const std::string& libPath) noexcept
	: _libPath(libPath)
{}

const std::string mcf::IR::IncludeLib::Inspect(void) const noexcept
{
	return "includelib " + _libPath;
}

mcf::IR::Extern::Extern(const std::string& name, const std::vector<mcf::Object::TypeInfo>& params, const bool hasVariadic) noexcept
	: _name(name)
	, _params(params)
	, _hasVariadic(hasVariadic)
{
#if defined(_DEBUG)
	const size_t size = _params.size();
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_params[i].IsValid(), u8"_params[%zu]가 유효하지 않습니다.", i);
		const size_t arraySize = _params[i].ArraySizeList.size();
		for (size_t j = 0; j < arraySize; ++j)
		{
			DebugAssert(_params[i].ArraySizeList[j] > 0, u8"_params[%zu].ArraySizeList[%zu]는 0이상 이어야 합니다. 값=%zu", i, j, _params[i].ArraySizeList[j]);
		}
	}
#endif
}

const std::string mcf::IR::Extern::Inspect(void) const noexcept
{
	std::string buffer = _name + " PROTO";
	if (_params.empty())
	{
		return _hasVariadic ? (buffer + " : VARARG") : buffer;
	}

	const size_t size = _params.size();
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_params[i].IsValid(), u8"_params[%zu]가 valid 하지 않습니다.", i);
		buffer += std::string(i == 0 ? " : " : ", ") + _params[i].Inspect();
	}
	return _hasVariadic ? (buffer + ", VARARG") : buffer;
}

mcf::IR::Program::Program(PointerVector&& objects) noexcept
	: _objects(std::move(objects))
{
#if defined(_DEBUG)
	const size_t size = _objects.size();
	DebugAssert(size != 0, u8"_objects에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_objects[i].get() != nullptr, u8"_objects[%zu]는 nullptr 여선 안됩니다.", i);
	}
#endif
}

const std::string mcf::IR::Program::Inspect(void) const noexcept
{
	std::string buffer;
	const size_t size = _objects.size();
	DebugAssert(size != 0, u8"_objects에 값이 최소 한개 이상 있어야 합니다.");
	for (size_t i = 0; i < size; i++)
	{
		DebugAssert(_objects[i].get() != nullptr, u8"_objects[%zu]는 nullptr 여선 안됩니다.", i);
		buffer += (i == 0 ? "" : "\n") + _objects[i]->Inspect();
	}
	return buffer;
}

mcf::IR::Expression::TypeIdentifier::TypeIdentifier(const mcf::Object::TypeInfo& info) noexcept
	: _info(info)
{
	DebugAssert(_info.IsValid(), u8"_info가 유효하지 않습니다.");
}

mcf::IR::Expression::GlobalVariableIdentifier::GlobalVariableIdentifier(const mcf::Object::Variable& variable) noexcept
	: _variable(variable)
{
	DebugAssert(_variable.IsValid(), u8"_variable가 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::GlobalVariableIdentifier::Inspect(void) const noexcept
{
	DebugMessage(u8"구현 필요");
	return std::string();
}

mcf::IR::Expression::LocalVariableIdentifier::LocalVariableIdentifier(const mcf::Object::Variable& info) noexcept
	: _info(info)
{
	DebugAssert(_info.IsValid(), u8"_info가 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::LocalVariableIdentifier::Inspect(void) const noexcept
{
	DebugMessage(u8"구현 필요");
	return std::string();
}

mcf::IR::Expression::FunctionIdentifier::FunctionIdentifier(const mcf::Object::FunctionInfo& info) noexcept
	: _info(info)
{
	DebugAssert(_info.IsValid(), u8"_info가 유효하지 않습니다.");
}

const std::string mcf::IR::Expression::FunctionIdentifier::Inspect(void) const noexcept
{
	DebugMessage(u8"구현 필요");
	return std::string();
}