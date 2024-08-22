#pragma once
#include <string>
#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <unordered_set>

#include <ast.h>
#include <object.h>

namespace mcf
{
	namespace Evaluator
	{
		class Object final
		{
		public:
			explicit Object(void) noexcept = default;
			explicit Object(const std::vector<mcf::Object::TypeInfo>& primitiveTypes) noexcept;

			mcf::IR::Pointer Eval(_Notnull_ const mcf::AST::Node::Interface* node) noexcept;

		private:
			mcf::IR::Pointer EvalProgram(_Notnull_ const mcf::AST::Program* program) noexcept;

			mcf::IR::Pointer EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement) noexcept;
			mcf::IR::Pointer EvalExternStatement(_Notnull_ const mcf::AST::Statement::Extern* statement) noexcept;

			mcf::Object::FunctionInfo EvalFunctionSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::FunctionSignature* intermediate) const noexcept;
			const bool EvalFunctionParamsIntermediate(_Out_ mcf::Object::FunctionParams& outParams, _Notnull_ const mcf::AST::Intermediate::FunctionParams* intermediate) const noexcept;
			mcf::Object::Variable EvalVariavbleSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::VariableSignature* intermediate) const noexcept;
			mcf::Object::TypeInfo EvalTypeSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::TypeSignature* intermediate) const noexcept;

			mcf::IR::Expression::Pointer EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression) const noexcept;
			mcf::IR::Expression::Pointer EvalIdentifierExpression(_Notnull_ const mcf::AST::Expression::Identifier* expression) const noexcept;

			inline const bool IsIdentifierRegistered(const std::string& name) const noexcept { return _allIdentifierSet.find(name) != _allIdentifierSet.end(); }

			const bool DefineType(const std::string& name, const mcf::Object::TypeInfo& info) noexcept;
			const mcf::Object::TypeInfo GetTypeInfo(const std::string& name) const noexcept;

			const bool DefineGlobalVariable(const std::string& name, const mcf::Object::Variable& info) noexcept;
			const mcf::Object::Variable GetGlobalVariable(const std::string& name) const noexcept;

			const bool DefineFunction(const std::string& name, const mcf::Object::FunctionInfo& info) noexcept;
			const mcf::Object::FunctionInfo GetFunction(const std::string& name) const noexcept;

		private:
			std::unordered_set<std::string> _allIdentifierSet;
			std::unordered_map<std::string, mcf::Object::TypeInfo> _typeInfoMap;
			std::unordered_map<std::string, mcf::Object::Variable> _globalVariables;
			std::unordered_map<std::string, mcf::Object::FunctionInfo> _functionInfoMap;
		};
	}
}