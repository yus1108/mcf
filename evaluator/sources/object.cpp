#include "pch.h"
#include "object.h"

const std::string mcf::Object::IndexData::Unknown::Inspect( void ) const noexcept
{
	DebugMessage(u8"Inspect 불가능한 오브젝트입니다.");
	return std::string();
}

mcf::Object::TypeInfo::TypeInfo(const bool isUnsigned, const std::string& typeName, std::vector<mcf::Object::IndexData::Pointer>&& indexDataList) noexcept
	: _isUnsigned(isUnsigned)
	, _typeName(typeName)
	, _indexDataList(std::move(indexDataList))
{
	DebugAssert(_typeName.empty() == false, u8"_typeName가 비어 있으면 안됩니다.");
#if defined(_DEBUG)
	const size_t indexDataListCount = _indexDataList.size();
	for (size_t i = 0; i < indexDataListCount; i++)
	{
		DebugAssert(_indexDataList[i].get() != nullptr, u8"_indexDataList[%zu]는 nullptr 여선 안됩니다.", i);
		DebugAssert(_indexDataList[i]->GetType() == mcf::Object::Type::INDEXDATA, u8"_indexDataList[%zu]는 index data 여야 합니다.", i);
	}
#endif
}

const std::string mcf::Object::TypeInfo::Inspect(void) const noexcept
{
	DebugMessage(u8"Inspect 불가능한 오브젝트입니다.");
	return std::string();
}

const int8_t mcf::Object::Integer::GetByte(void) const noexcept
{
	DebugAssert(IsByte(), u8"");
	return 0;
}

mcf::Object::Program::Program(PointerVector&& objects) noexcept
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

const std::string mcf::Object::Program::Inspect(void) const noexcept
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

mcf::Object::IncludeLib::IncludeLib(const std::string& libPath) noexcept
	: _libPath(libPath)
{}

const std::string mcf::Object::IncludeLib::Inspect(void) const noexcept
{
	return "includelib " + _libPath;
}

mcf::Object::Extern::Extern(const std::string& functionName, const std::vector<std::string>& paramTypes) noexcept
	: _functionName(functionName)
	, _paramTypes(paramTypes)
{
	DebugAssert(_functionName.empty() == false, u8"_functionName가 비어 있으면 안됩니다.");
#if defined(_DEBUG)
	const size_t count = _paramTypes.size();
	for (size_t i = 0; i < count; i++)
	{
		DebugAssert(_paramTypes[i].empty() != false, u8"_paramTypes[%zu]는 비어 있으면 안됩니다.", i);
	}
#endif
}

const std::string mcf::Object::Extern::Inspect(void) const noexcept
{
	DebugAssert(_functionName.empty() == false, u8"_functionName가 비어 있으면 안됩니다.");

	std::string buffer = _functionName + " PROTO ";
	if (_paramTypes.empty())
	{
		return buffer;
	}

	buffer += " : ";

	const size_t count = _paramTypes.size();
	for (size_t i = 0; i < count; i++)
	{
		DebugAssert(_paramTypes[i].empty() != false, u8"_paramTypes[%zu]는 비어 있으면 안됩니다.", i);
		buffer += _paramTypes[i] + (i == count - 1 ? ", " : "");
	}
	return std::string();
}