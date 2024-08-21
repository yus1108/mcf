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
		class FunctionInfo final
		{
		public:
			explicit FunctionInfo(void) noexcept = default;
			explicit FunctionInfo(	std::string name,
									std::vector<std::string> paramNames,
									std::vector<std::string> paramTypes,
									std::vector<bool> paramIsUnsignedList,
									std::vector<bool> paramIsVariadicList,
									std::pair<bool, std::string> returnType) noexcept;
			explicit FunctionInfo(	std::string name,
									std::initializer_list<std::string> paramNames,
									std::initializer_list<std::string> paramTypes,
									std::initializer_list<bool> paramIsUnsignedList,
									std::initializer_list<bool> paramIsVariadicList,
									std::pair<bool, std::string> returnType) noexcept;

			inline friend bool operator==(const FunctionInfo& lhs, const FunctionInfo& rhs);

			inline const std::string& GetName(void) const noexcept { return _name;}
			const std::string ConvertToString(void) const noexcept;

		private:
			std::string _name;
			std::vector<std::string> _paramNames;
			std::vector<std::string> _paramTypes;
			std::vector<bool> _paramIsUnsignedList;
			std::vector<bool> _paramIsVariadicList;
			std::pair<bool, std::string> _returnType;
		};
		inline bool operator==(const FunctionInfo& lhs, const FunctionInfo& rhs) 
		{ 
			return (lhs._name == rhs._name) && 
				(lhs._paramNames == rhs._paramNames) &&
				(lhs._paramTypes == rhs._paramTypes) &&
				(lhs._paramIsUnsignedList == rhs._paramIsUnsignedList) &&
				(lhs._paramIsVariadicList == rhs._paramIsVariadicList) &&
				(lhs._returnType == rhs._returnType);
		}

		class Object final
		{
		public:
			mcf::Object::Pointer Eval(_Notnull_ const mcf::AST::Node::Interface* node) const noexcept;
			const mcf::Evaluator::FunctionInfo FindFunctionInfo(const std::string& functionName) const noexcept;

		private:
			mcf::Object::Pointer EvalProgram(_Notnull_ const mcf::AST::Program* program) const noexcept;

			mcf::Object::Pointer EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement) const noexcept;
			mcf::Object::Extern::Pointer EvalExternStatement(_Notnull_ const mcf::AST::Statement::Extern* statement) const noexcept;

			mcf::Object::Pointer EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression) const noexcept;

		private:
			std::unordered_map<std::string, FunctionInfo> _functionMap;
		};
	}
}