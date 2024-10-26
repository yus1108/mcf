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
		class MemoryAllocator final
		{
		public:
			static const size_t DEFAULT_ALIGNMENT = 16;

		public:
			void AddSize(const size_t size) noexcept;
			const bool IsEmpty(void) const noexcept;
			inline const size_t GetCount(void) const noexcept { return _sizes.size(); }
			inline const size_t GetOffset(const size_t index) const noexcept { return _offsets[index]; }
			inline const size_t GetTotalSize(void) const noexcept { return IsEmpty() ? 0 : (_offsets.back() + _sizes.back() + _paddings.back()); }
			inline void Clear(void) noexcept { _sizes.clear(); _paddings.clear(); _offsets.clear(); _alignment = DEFAULT_ALIGNMENT; }

		private:
			void Realign(const size_t alignment) noexcept;

		private:
			std::vector<size_t> _sizes;
			std::vector<size_t> _paddings;
			std::vector<size_t> _offsets;
			size_t _alignment = DEFAULT_ALIGNMENT;
		};

		class FunctionIRGenerator;
		class FunctionCallIRGenerator final
		{
		public:
			explicit FunctionCallIRGenerator(void) noexcept = delete;
			explicit FunctionCallIRGenerator(const mcf::Object::FunctionInfo& info) noexcept;

			void AddVariadicMemory(_Notnull_ const mcf::IR::Expression::Interface* expression) noexcept;
			void FinishAddingVariadicMemory(void) noexcept;

			const bool AddParameter(_Notnull_ const mcf::IR::Expression::Interface* expression, _Notnull_ const FunctionIRGenerator* functionGenerator) noexcept;
			const bool AddParameter(const mcf::IR::ASM::SizeOf& source) noexcept;
			const bool AddUnsafePointerParameter(_Notnull_ const mcf::IR::Expression::String* stringExpression) noexcept;
			const bool AddUnsafePointerParameter(const mcf::IR::ASM::UnsafePointerAddress& source) noexcept;
			void AddGeneratedIRCode(_Out_ mcf::IR::ASM::PointerVector& outVector) noexcept;

		private:
			const size_t GetReservedMemory(void) const noexcept;

			void AddPointerParameterInternal(const mcf::IR::ASM::UnsafePointerAddress& source) noexcept;
			const bool AddUnsafePointerParameterInternal(_Notnull_ const mcf::IR::Expression::Interface* expression, _Notnull_ const FunctionIRGenerator* functionGenerator) noexcept;


		private:
			const mcf::Object::FunctionInfo _info;
			MemoryAllocator _localMemory;
			mcf::IR::ASM::PointerVector _localCodes;
			size_t _currParamIndex = 0;
			bool _isNeedToAddVariadicMemory = false;
		};

		class FunctionIRGenerator final
		{
			friend FunctionCallIRGenerator;
		public:
			explicit FunctionIRGenerator(void) noexcept = delete;
			explicit FunctionIRGenerator(const mcf::Object::FunctionInfo& info) noexcept;

			void AddLetStatement(_Notnull_ const mcf::IR::Let* object, _Notnull_ mcf::Object::Scope* scope) noexcept;
			void AddExpressionObject(_Notnull_ const mcf::IR::Expression::Interface* object, _Notnull_ const mcf::Object::Scope* scope) noexcept;
			void AddReturnStatement(_Notnull_ const mcf::IR::Return* object) noexcept;
			mcf::IR::ASM::PointerVector GenerateIRCode(void) noexcept;

		private:
			const size_t GetLocalVariableOffset(const std::string& variableName) const noexcept;

		private:
			static const size_t PARAM_OFFSET = 0x10;
			static const std::string INTERNAL_COPY_MEMORY_NAME;

			MemoryAllocator _localMemory;
			mcf::IR::ASM::PointerVector _localCodes;
			mcf::IR::ASM::PointerVector _beginCodes;
			mcf::IR::ASM::PointerVector _endCodes;
			mcf::Object::TypeInfo _returnType;
			std::unordered_map<std::string, size_t> _localVariableIndicesMap;
			std::unordered_map<std::string, size_t> _paramOffsetMap;
		};

		class Object final
		{
		public:
			explicit Object(void) noexcept = default;

			static const bool ValidateVariableTypeAndValue(const mcf::Object::Variable& variable, _Notnull_ const mcf::IR::Expression::Interface* value) noexcept;

			mcf::IR::Program::Pointer EvalProgram(_Notnull_ const mcf::AST::Program* program, _Notnull_ mcf::Object::Scope* scope) noexcept;

		private:

			mcf::IR::Pointer EvalStatement(_Notnull_ const mcf::AST::Statement::Interface* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalTypedefStatement(_Notnull_ const mcf::AST::Statement::Typedef* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalExternStatement(_Notnull_ const mcf::AST::Statement::Extern* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalLetStatement(_Notnull_ const mcf::AST::Statement::Let* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalUnusedStatement(_Notnull_ const mcf::AST::Statement::Unused* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalReturnStatement(_Notnull_ const mcf::AST::Statement::Return* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalFuncStatement(_Notnull_ const mcf::AST::Statement::Func* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalMainStatement(_Notnull_ const mcf::AST::Statement::Main* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalWhileStatement(_Notnull_ const mcf::AST::Statement::While* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::Pointer EvalAssignExpressionStatement(_Notnull_ const mcf::AST::Statement::AssignExpression* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;

			mcf::IR::PointerVector EvalBlockStatement(_Notnull_ const mcf::AST::Statement::Block* statement, _Notnull_ mcf::Object::Scope* scope) noexcept;
			mcf::IR::ASM::PointerVector EvalFunctionBlockStatement(const mcf::Object::FunctionInfo& info, _Notnull_ const mcf::AST::Statement::Block* statement) noexcept;

			mcf::Object::FunctionInfo EvalFunctionSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::FunctionSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			const bool EvalFunctionParamsIntermediate(_Out_ mcf::Object::FunctionParams& outParams, _Notnull_ const mcf::AST::Intermediate::FunctionParams* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::Object::Variable EvalVariavbleSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::VariableSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::Object::TypeInfo EvalTypeSignatureIntermediate(_Notnull_ const mcf::AST::Intermediate::TypeSignature* intermediate, _Notnull_ mcf::Object::Scope* scope) const noexcept;

			mcf::IR::Expression::Pointer EvalExpression(_Notnull_ const mcf::AST::Expression::Interface* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalIdentifierExpression(_Notnull_ const mcf::AST::Expression::Identifier* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalIntegerExpression(_Notnull_ const mcf::AST::Expression::Integer* expression, _Notnull_ const mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalStringExpression(_Notnull_ const mcf::AST::Expression::String* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalCallExpression(_Notnull_ const mcf::AST::Expression::Call* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalAsExpression(_Notnull_ const mcf::AST::Expression::As* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalIndexExpression(_Notnull_ const mcf::AST::Expression::Index* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalInitializerExpression(_Notnull_ const mcf::AST::Expression::Initializer* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			mcf::IR::Expression::Pointer EvalInfixExpression(_Notnull_ const mcf::AST::Expression::Infix* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;

			mcf::Object::FunctionInfo BuildFunctionInfo(const std::string& name, _Notnull_ const mcf::AST::Intermediate::FunctionParams* functionParams, const mcf::AST::Intermediate::TypeSignature* returnType, _Notnull_ mcf::Object::Scope* scope) const noexcept;

			mcf::Object::TypeInfo MakeArrayTypeInfo(_In_ mcf::Object::TypeInfo info, _Notnull_ const mcf::IR::Expression::Interface* index) const noexcept;
			void DetermineUnknownArrayIndex(_Inout_ mcf::Object::Variable& variable, _Notnull_ const mcf::IR::Expression::Interface* expression, _Notnull_ mcf::Object::Scope* scope) const noexcept;
			const std::vector<size_t> CalculateMaximumArrayIndex(_Notnull_ const mcf::IR::Expression::Interface* expression) const noexcept;
		};
	}
}