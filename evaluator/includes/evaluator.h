#pragma once
#include <ast.h>
#include <object.h>

namespace mcf
{
	namespace Evaluator
	{
		class Object final
		{
		public:
			mcf::Object::Pointer Eval(_Notnull_ const mcf::AST::Node::Interface* node) const noexcept;

		private:
			mcf::Object::Pointer EvalProgram(_Notnull_ const mcf::AST::Program* program) const noexcept;
			mcf::Object::Pointer EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement) const noexcept;
			mcf::Object::Pointer EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression) const noexcept;

		};
	}
}