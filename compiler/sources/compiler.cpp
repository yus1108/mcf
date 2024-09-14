#include "pch.h"
#include "compiler.h"

mcf::ASM::PointerVector mcf::Compiler::Object::GenerateCodes(const mcf::IR::Interface* irCodes, const mcf::Object::ScopeTree* scopeTree) noexcept
{
	MCF_UNUSED(irCodes, scopeTree);
	MCF_DEBUG_TODO(u8"구현 필요");
	return mcf::ASM::PointerVector();
}