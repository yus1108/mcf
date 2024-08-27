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
		class FunctionIRGenerator final
		{
		public:
			explicit FunctionIRGenerator(const mcf::Object::FunctionInfo& info) noexcept;

			const bool AddLocalVariable(_Notnull_ const mcf::IR::Let* object) noexcept;
			mcf::IR::PointerVector GenerateIRCode(void) const noexcept;

		private:

		};

		class Object final
		{
		public:
			explicit Object(void) noexcept = default;

			mcf::IR::Pointer Eval(_Notnull_ const mcf::AST::Node::Interface* node, _Notnull_ mcf::Object::Scope* scope) noexcept;

		private:
			mcf::IR::Pointer EvalProgram(_Notnull_ const mcf::AST::Program* program, _Notnull_ mcf::Object::Scope* scope) noexcept;

			mcf::IR::Pointer EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalExternStatement(_Notnull_ const mcf::AST::Statement::Extern* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalLetStatement(_Notnull_ const mcf::AST::Statement::Let* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalFuncStatement(_Notnull_ const mcf::AST::Statement::Func* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;

			mcf::IR::PointerVector EvalFunctionBlockStatement(const mcf::Object::FunctionInfo& info, _Notnull_ const mcf::AST::Statement::Block* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;

			mcf::Object::FunctionInfo EvalFunctionSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::FunctionSignature* intermediate, _Notnull_ const mcf::Object::Scope* scope) const noexcept;
			const bool EvalFunctionParamsIntermediate(_Out_ mcf::Object::FunctionParams& outParams, _Notnull_ const mcf::AST::Intermediate::FunctionParams* intermediate, _Notnull_ const mcf::Object::Scope* scope) const noexcept;
			mcf::Object::Variable EvalVariavbleSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::VariableSignature* intermediate, _Notnull_ const mcf::Object::Scope* scope) const noexcept;
			mcf::Object::TypeInfo EvalTypeSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::TypeSignature* intermediate, _Notnull_ const mcf::Object::Scope* scope) const noexcept;

			mcf::IR::Expression::Pointer EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalIdentifierExpression(_Notnull_ const mcf::AST::Expression::Identifier* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalIntegerExpression(_Notnull_ const mcf::AST::Expression::Integer* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalIndexExpression(_Notnull_ const mcf::AST::Expression::Index* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalInitializerExpression(_Notnull_ const mcf::AST::Expression::Initializer* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept;

			mcf::Object::TypeInfo MakeArrayTypeInfo(_Notnull_ mcf::Object::TypeInfo info, _Notnull_ const mcf::IR::Expression::Interface* index) const noexcept;
			const bool ValidateVariableTypeAndValue(_Notnull_ mcf::Object::VariableInfo info, _Notnull_ const mcf::IR::Expression::Interface* value) const noexcept;
		};
	}
}