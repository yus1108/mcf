#include "pch.h"
#include "object.h"

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

const int8_t mcf::Object::Integer::GetByte(void) const noexcept
{
	DebugAssert(IsByte(), "");
	return 0;
}