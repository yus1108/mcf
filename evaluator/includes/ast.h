﻿#pragma once
#include <memory>
#include <string>
#include <vector>
#include <lexer.h>

namespace mcf
{
	namespace AST
	{
		namespace Node
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				EXPRESSION,
				INTERMEDIATE,
				STATEMENT,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT,
			};

			class Interface
			{
			public:
				virtual ~Interface(void) noexcept = default;
				virtual const mcf::AST::Node::Type	GetNodeType(void) const noexcept = 0;
				virtual const std::string ConvertToString(void) const noexcept = 0;
			};

			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;
		}

		namespace Expression
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				IDENTIFIER,
				INTEGER,
				STRING,
				PREFIX,
				GROUP,
				INFIX,
				CALL,
				INDEX,
				INITIALIZER,
				MAP_INITIALIZER,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT,
			};

			class Interface : public mcf::AST::Node::Interface
			{
			public:
				virtual const Type GetExpressionType(void) const noexcept = 0;
				inline virtual const mcf::AST::Node::Type GetNodeType(void) const noexcept override final
				{
					return mcf::AST::Node::Type::EXPRESSION;
				}
			};

			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;

			class Invalid : public Interface
			{
			public:
				inline static Pointer Make() { return std::make_unique<Invalid>(); }
				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::INVALID; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return "<Invalid>"; }
			};

			class Identifier : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Identifier>;
				using PointerVector = std::vector<Pointer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Identifier>(std::move(args)...); }

			public:
				explicit Identifier(void) noexcept = default;
				explicit Identifier(const mcf::Token::Data& token) noexcept : _token(token) {}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::IDENTIFIER; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return "<Identifier: " + _token.Literal + ">"; }

			private:
				mcf::Token::Data _token;
			};

			class Integer : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Integer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Integer>(std::move(args)...); }

			public:
				explicit Integer(void) noexcept = default;
				explicit Integer(const mcf::Token::Data& token) noexcept : _token(token) {}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::INTEGER; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return "<Integer: " + _token.Literal + ">"; }

			private:
				mcf::Token::Data _token;
			};

			class String : public Interface
			{
			public:
				using Pointer = std::unique_ptr<String>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<String>(std::move(args)...); }

			public:
				explicit String(void) noexcept = default;
				explicit String(const mcf::Token::Data& token) noexcept : _token(token) {}

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::STRING; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return "<String: " + _token.Literal + ">"; }

			private:
				mcf::Token::Data _token;
			};

			class Prefix : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Prefix>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Prefix>(std::move(args)...); }

			public:
				explicit Prefix(void) noexcept = default;
				explicit Prefix(const mcf::Token::Data& prefixOperator, mcf::AST::Expression::Pointer&& right) noexcept;

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::PREFIX; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::Token::Data _prefixOperator;
				mcf::AST::Expression::Pointer _right;
			};

			class Group : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Group>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Group>(std::move(args)...); }

			public:
				explicit Group(void) noexcept = default;
				explicit Group(mcf::AST::Expression::Pointer&& expression ) noexcept;

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::GROUP; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Pointer _expression;
			};

			class Infix : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Infix>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Infix>(std::move(args)...); }

			public:
				explicit Infix(void) noexcept = default;
				explicit Infix(mcf::AST::Expression::Pointer&& left, const mcf::Token::Data& infixOperator, mcf::AST::Expression::Pointer&& right) noexcept;

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::INFIX; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::Token::Data _infixOperator;
				mcf::AST::Expression::Pointer _left;
				mcf::AST::Expression::Pointer _right;
			};

			class Call : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Call>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Call>(std::move(args)...); }

			public:
				explicit Call(void) noexcept = default;
				explicit Call(mcf::AST::Expression::Pointer&& left, mcf::AST::Expression::PointerVector&& params) noexcept;

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::CALL; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Pointer _left;
				mcf::AST::Expression::PointerVector _params;
			};

			class Index : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Index>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Index>(std::move(args)...); }

			public:
				explicit Index(void) noexcept = default;
				explicit Index(mcf::AST::Expression::Pointer&& left, mcf::AST::Expression::Pointer&& index) noexcept;

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::INDEX; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Pointer _left;
				mcf::AST::Expression::Pointer _index;
			};

			class Initializer : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Initializer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Initializer>(std::move(args)...); }

			public:
				explicit Initializer(void) noexcept = default;
				explicit Initializer(PointerVector&& keyList) noexcept;

				inline virtual const Type GetExpressionType(void) const noexcept override { return Type::INITIALIZER; }
				virtual const std::string ConvertToString(void) const noexcept override;

			protected:
				PointerVector _keyList;
			};

			class MapInitializer : public Initializer
			{
			public:
				using Pointer = std::unique_ptr<MapInitializer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<MapInitializer>(std::move(args)...); }

			public:
				explicit MapInitializer(void) noexcept = default;
				explicit MapInitializer(PointerVector&& keyist, PointerVector&& valueList) noexcept;

				inline virtual const Type GetExpressionType(void) const noexcept override final { return Type::MAP_INITIALIZER; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				PointerVector _valueList;
			};
		}

		namespace Intermediate
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				VARIADIC,
				TYPE_SIGNATURE,
				VARIABLE_SIGNATURE,
				FUNCTION_SIGNATURE,
				FUNCTION_PARAMS,
				STATEMENTS,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT,
			};

			class Interface : public mcf::AST::Node::Interface
			{
			public:
				virtual const Type GetIntermediateType(void) const noexcept = 0;
				inline virtual const mcf::AST::Node::Type GetNodeType(void) const noexcept override final
				{
					return mcf::AST::Node::Type::INTERMEDIATE;
				}
			};

			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;

			class Invalid : public Interface
			{
			public:
				inline static Pointer Make(void) { return std::make_unique<Invalid>(); }
				inline virtual const Type GetIntermediateType(void) const noexcept override final { return Type::INVALID; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return "<Invalid>"; }
			};


			class Variadic : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Variadic>;

				template <class... VariadicTemplateClass>
				inline static Pointer Make(VariadicTemplateClass&& ...args) { return std::make_unique<Variadic>(std::move(args)...); }

			public:
				explicit Variadic(void) noexcept = default;
				explicit Variadic(mcf::AST::Expression::Identifier::Pointer&& name) noexcept;

				inline virtual const Type GetIntermediateType(void) const noexcept override final { return Type::VARIADIC; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Identifier::Pointer _name;
			};

			class TypeSignature : public Interface
			{
			public:
				using Pointer = std::unique_ptr<TypeSignature>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<TypeSignature>(std::move(args)...); }

			public:
				explicit TypeSignature(void) noexcept = default;
				explicit TypeSignature(const bool isUnsigned, mcf::AST::Expression::Pointer&& signature) noexcept;

				inline virtual const Type GetIntermediateType(void) const noexcept override final { return Type::TYPE_SIGNATURE; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				bool _isUnsigned;
				mcf::AST::Expression::Pointer _signature;
			};

			class VariableSignature : public Interface
			{
			public:
				using Pointer = std::unique_ptr<VariableSignature>;
				using PointerVector = std::vector<Pointer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<VariableSignature>(std::move(args)...); }

			public:
				explicit VariableSignature(void) noexcept = default;
				explicit VariableSignature(mcf::AST::Expression::Identifier::Pointer&& name, TypeSignature::Pointer&& typeSignature) noexcept;
				
				inline virtual const Type GetIntermediateType(void) const noexcept override final { return Type::VARIABLE_SIGNATURE; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Identifier::Pointer _name;
				TypeSignature::Pointer _typeSignature;
			};

			class FunctionParams : public Interface
			{
			public:
				using Pointer = std::unique_ptr<FunctionParams>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<FunctionParams>(std::move(args)...); }

			public:
				explicit FunctionParams(void) noexcept = default;
				explicit FunctionParams(std::vector<VariableSignature::Pointer>&& params, Variadic::Pointer&& variadic) noexcept
					: _params(std::move(params)), _variadic(std::move(variadic)) {}

				inline const bool IsVoid(void) const noexcept { return HasParams() == false && HasVariadic() == false; }
				inline const bool HasParams(void) const noexcept { return _params.size() != 0; }
				inline const bool HasVariadic(void) const noexcept { return _variadic.get() != nullptr; }

				inline virtual const Type GetIntermediateType(void) const noexcept override final { return Type::FUNCTION_PARAMS; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				VariableSignature::PointerVector _params;
				Variadic::Pointer _variadic;
			};

			class FunctionSignature : public Interface
			{
			public:
				using Pointer = std::unique_ptr<FunctionSignature>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<FunctionSignature>(std::move(args)...); }

			public:
				explicit FunctionSignature(void) noexcept = default;
				explicit FunctionSignature(mcf::AST::Expression::Identifier::Pointer name, FunctionParams::Pointer params, TypeSignature::Pointer returnType) noexcept;

				inline const bool IsReturnVoid(void) const noexcept { return _returnType.get() == nullptr; }

				inline virtual const Type GetIntermediateType(void) const noexcept override final { return Type::FUNCTION_SIGNATURE; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Identifier::Pointer _name;
				FunctionParams::Pointer _params;
				TypeSignature::Pointer _returnType;
			};
		}

		namespace Statement
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				INCLUDE_LIBRARY,
				TYPEDEF,
				EXTERN,
				LET, 
				BLOCK,
				RETURN,
				FUNC,
				MAIN,
				EXPRESSION,
				UNUSED,

				// 이 밑으로는 수정하면 안됩니다.
				COUNT,
			};

			class Interface : public mcf::AST::Node::Interface
			{
			public:
				virtual const Type GetStatementType(void) const noexcept = 0;
				inline virtual const mcf::AST::Node::Type GetNodeType(void) const noexcept override final 
				{ 
					return mcf::AST::Node::Type::STATEMENT;
				}
			};
			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;

			class Invalid : public Interface
			{
			public:
				inline static Pointer Make() { return std::make_unique<Invalid>(); }
				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::INVALID; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return "<Invalid>"; }
			};

			class IncludeLibrary : public Interface
			{
			public:
				using Pointer = std::unique_ptr<IncludeLibrary>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<IncludeLibrary>(std::move(args)...); }

			public:
				explicit IncludeLibrary(void) noexcept = default;
				explicit IncludeLibrary(mcf::Token::Data libPath) noexcept : _libPath(libPath) {}

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::INCLUDE_LIBRARY; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::Token::Data _libPath;
				const bool _hasHeader = false;
			};

			class Typedef : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Typedef>;
				using SignaturePointer = mcf::AST::Intermediate::VariableSignature::Pointer;
				using BindMapPointer = mcf::AST::Expression::MapInitializer::Pointer;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Typedef>(std::move(args)...); }

			public:
				explicit Typedef(void) noexcept = default;
				explicit Typedef(SignaturePointer&& signature, BindMapPointer&& bindMap) noexcept;

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::INCLUDE_LIBRARY; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				SignaturePointer _signature;
				BindMapPointer _bindMap;
			};

			class Extern : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Extern>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Extern>(std::move(args)...); }

			public:
				explicit Extern(void) noexcept = default;
				explicit Extern(const bool isAssemblyFunction, mcf::AST::Intermediate::FunctionSignature::Pointer&& signature) noexcept;

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::EXTERN; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				bool _isAssemblyFunction;
				mcf::AST::Intermediate::FunctionSignature::Pointer _signature;
			};

			class Let : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Let>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Let>(std::move(args)...); }

			public:
				explicit Let(void) noexcept = default;
				explicit Let(mcf::AST::Intermediate::VariableSignature::Pointer&& signature, mcf::AST::Expression::Pointer&& expression) noexcept;

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::LET; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Intermediate::VariableSignature::Pointer _signature;
				mcf::AST::Expression::Pointer _expression;
			};

			class Block : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Block>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Block>(std::move(args)...); }

			public:
				explicit Block(void) noexcept = default;
				explicit Block(Statement::PointerVector&& statements) noexcept;

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::BLOCK; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				Statement::PointerVector _statements;
			};

			class Return : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Return>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Return>(std::move(args)...); }

			public:
				explicit Return(void) noexcept = default;
				explicit Return(mcf::AST::Expression::Pointer&& returnValue) noexcept;

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::BLOCK; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Pointer _returnValue;;
			};

			class Func : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Func>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Func>(std::move(args)...); }

			public:
				explicit Func(void) noexcept = default;
				explicit Func(mcf::AST::Intermediate::FunctionSignature::Pointer&& signature, mcf::AST::Statement::Block::Pointer&& block) noexcept;

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::BLOCK; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Intermediate::FunctionSignature::Pointer _signature;;
				mcf::AST::Statement::Block::Pointer _block;
			};

			class Main : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Main>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Main>(std::move(args)...); }

			public:
				explicit Main(void) noexcept = default;
				explicit Main(mcf::AST::Intermediate::FunctionParams::Pointer&& params, mcf::AST::Intermediate::TypeSignature::Pointer&& returnType, Block::Pointer&& block) noexcept;

				inline const bool IsReturnVoid(void) const noexcept { return _returnType.get() == nullptr; }

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::MAIN; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Intermediate::FunctionParams::Pointer _params;;
				mcf::AST::Intermediate::TypeSignature::Pointer _returnType;
				Block::Pointer _block;
			};

			class Expression : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Expression>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Expression>(std::move(args)...); }

			public:
				explicit Expression(void) noexcept = default;
				explicit Expression(mcf::AST::Expression::Pointer&& expression) noexcept;

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::EXPRESSION; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Pointer _expression;
			};

			class Unused : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Unused>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Unused>(std::move(args)...); }

			public:
				explicit Unused(void) noexcept = default;
				explicit Unused(mcf::AST::Expression::Identifier::PointerVector&& identifiers) noexcept;

				inline virtual const Type GetStatementType(void) const noexcept override final { return Type::UNUSED; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Identifier::PointerVector _identifiers;
			};
		}

		class Program final
		{
		public:
			explicit Program(void) noexcept = default;
			explicit Program(mcf::AST::Statement::PointerVector&& statements) noexcept;

			inline const size_t GetStatementCount(void) const noexcept { return _statements.size(); }
			inline const mcf::AST::Statement::Interface* GetUnsafeStatementPointerAt(const size_t index) const noexcept 
			{
				return _statements[index].get(); 
			}

			const std::string ConvertToString(void) const noexcept;

		private:
			mcf::AST::Statement::PointerVector _statements;
		};
	}
}