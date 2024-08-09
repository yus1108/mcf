#pragma once
#include <memory>
#include <string>
#include <vector>
#include <parser/includes/lexer.h>

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
				INDEX,

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
				using Pointer = std::unique_ptr<Invalid>;
				virtual const Type GetExpressionType(void) const noexcept override final { return Type::INVALID; }
			};

			class Identifier : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Identifier>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Identifier>(std::move(args)...); }

			public:
				explicit Identifier(void) noexcept = default;
				explicit Identifier(const mcf::Token::Data& token) noexcept : _token(token) {}

				virtual const Type GetExpressionType(void) const noexcept override final { return Type::IDENTIFIER; }
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

				virtual const Type GetExpressionType(void) const noexcept override final { return Type::INTEGER; }
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

				virtual const Type GetExpressionType(void) const noexcept override final { return Type::STRING; }
				inline virtual const std::string ConvertToString(void) const noexcept override final { return "<String: " + _token.Literal + ">"; }

			private:
				mcf::Token::Data _token;
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

				virtual const Type GetExpressionType(void) const noexcept override final { return Type::INDEX; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Pointer _left;
				mcf::AST::Expression::Pointer _index;
			};
		}

		namespace Intermediate
		{
			enum class Type : unsigned char
			{
				INVALID = 0,

				MAP_INITIALIZER,
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

			class Invalid : public Interface
			{
			public:
				using Pointer = std::unique_ptr<Invalid>;
				virtual const Type GetIntermediateType(void) const noexcept override final { return Type::INVALID; }
			};

			using Pointer = std::unique_ptr<Interface>;
			using PointerVector = std::vector<Pointer>;

			class MapInitializer : public Interface
			{
			public:
				using KeyValue = std::pair<mcf::AST::Expression::Pointer, mcf::AST::Expression::Pointer>;
				using Pointer = std::unique_ptr<MapInitializer>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<MapInitializer>(std::move(args)...); }

			public:
				explicit MapInitializer(void) noexcept = default;
				explicit MapInitializer(std::vector<KeyValue>&& itemList) noexcept;

				virtual const Type GetIntermediateType(void) const noexcept override final { return Type::MAP_INITIALIZER; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				std::vector<KeyValue> _itemList;
			};

			class TypeSignature : public Interface
			{
			public:
				using Pointer = std::unique_ptr<TypeSignature>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<TypeSignature>(std::move(args)...); }

			public:
				explicit TypeSignature(void) noexcept = default;
				explicit TypeSignature(mcf::AST::Expression::Pointer&& signature) noexcept;

				virtual const Type GetIntermediateType(void) const noexcept override final { return Type::TYPE_SIGNATURE; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Pointer _signature;
			};

			class VariableSignature : public Interface
			{
			public:
				using Pointer = std::unique_ptr<VariableSignature>;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<VariableSignature>(std::move(args)...); }

			public:
				explicit VariableSignature(void) noexcept = default;
				explicit VariableSignature(mcf::AST::Expression::Identifier::Pointer&& name, TypeSignature::Pointer&& typeSignature) noexcept;
				
				virtual const Type GetIntermediateType(void) const noexcept override final { return Type::VARIABLE_SIGNATURE; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				mcf::AST::Expression::Identifier::Pointer _name;
				TypeSignature::Pointer _typeSignature;
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
				FUNC,
				RETURN,

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
				using Pointer = std::unique_ptr<Invalid>;
				virtual const Type GetStatementType(void) const noexcept override final { return Type::INVALID; }
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

				virtual const Type GetStatementType(void) const noexcept override final { return Type::INCLUDE_LIBRARY; }
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
				using BindMapPointer = mcf::AST::Intermediate::MapInitializer::Pointer;

				template <class... Variadic>
				inline static Pointer Make(Variadic&& ...args) { return std::make_unique<Typedef>(std::move(args)...); }

			public:
				explicit Typedef(void) noexcept = default;
				explicit Typedef(SignaturePointer&& signature, BindMapPointer&& bindMap) noexcept;

				virtual const Type GetStatementType(void) const noexcept override final { return Type::INCLUDE_LIBRARY; }
				virtual const std::string ConvertToString(void) const noexcept override final;

			private:
				SignaturePointer _signature;
				BindMapPointer _bindMap;
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