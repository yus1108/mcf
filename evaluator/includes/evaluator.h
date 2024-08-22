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
			explicit Object(const std::vector<TypeInfo>& primitiveTypes) noexcept;

			mcf::Object::Pointer Eval(_Notnull_ const mcf::AST::Node::Interface* node) noexcept;

		private:
			mcf::Object::Pointer EvalProgram(_Notnull_ const mcf::AST::Program* program) noexcept;

			mcf::Object::Pointer EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement) noexcept;
			mcf::Object::Pointer EvalExternStatement(_Notnull_ const mcf::AST::Statement::Extern* statement) noexcept;

			mcf::Evaluator::FunctionInfo EvalFunctionSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::FunctionSignature* intermediate) const noexcept;
			const bool EvalFunctionParamsIntermediate(_Out_ mcf::Evaluator::FunctionParams& outParams, _Notnull_ const mcf::AST::Intermediate::FunctionParams* intermediate) const noexcept;
			mcf::Evaluator::Variable EvalVariavbleSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::VariableSignature* intermediate) const noexcept;
			mcf::Evaluator::TypeInfo EvalTypeSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::TypeSignature* intermediate) const noexcept;

			mcf::Object::Expression::Pointer EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression) const noexcept;
			mcf::Object::Expression::Pointer EvalIdentifierExpression(_Notnull_ const mcf::AST::Expression::Identifier* expression) const noexcept;

			inline const bool IsIdentifierRegistered(const std::string& name) const noexcept { return _allIdentifierSet.find(name) != _allIdentifierSet.end(); }

			const bool DefineType(const std::string& name, const TypeInfo& info) noexcept;
			const mcf::Evaluator::TypeInfo GetTypeInfo(const std::string& name) const noexcept;

			const bool DefineGlobalVariable(const std::string& name, const Variable& info) noexcept;
			const mcf::Evaluator::Variable GetGlobalVariable(const std::string& name) const noexcept;

			const bool DefineFunction(const std::string& name, const FunctionInfo& info) noexcept;
			const mcf::Evaluator::FunctionInfo GetFunction(const std::string& name) const noexcept;

		private:
			std::unordered_set<std::string> _allIdentifierSet;
			std::unordered_map<std::string, TypeInfo> _typeInfoMap;
			std::unordered_map<std::string, Variable> _globalVariables;
			std::unordered_map<std::string, FunctionInfo> _functionInfoMap;
		};
	}
}