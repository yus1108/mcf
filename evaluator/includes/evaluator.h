#pragma once
#include <string>
#include <vector>
#include <initializer_list>

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

			mcf::IR::Pointer Eval(_Notnull_ const mcf::AST::Node::Interface* node, _Notnull_ mcf::Object::Scope* scope) noexcept;

		private:
			mcf::IR::Pointer EvalProgram(_Notnull_ const mcf::AST::Program* program, _Notnull_ mcf::Object::Scope* scope) noexcept;

			mcf::IR::Pointer EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalExternStatement(_Notnull_ const mcf::AST::Statement::Extern* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;

			mcf::Object::FunctionInfo EvalFunctionSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::FunctionSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			const bool EvalFunctionParamsIntermediate(_Out_ mcf::Object::FunctionParams& outParams, _Notnull_ const mcf::AST::Intermediate::FunctionParams* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::Object::Variable EvalVariavbleSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::VariableSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::Object::TypeInfo EvalTypeSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::TypeSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept;

			mcf::IR::Expression::Pointer EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalIdentifierExpression(_Notnull_ const mcf::AST::Expression::Identifier* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalIndexExpression(_Notnull_ const mcf::AST::Expression::Index* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
		};
	}
}