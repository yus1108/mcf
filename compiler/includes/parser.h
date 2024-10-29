#pragma once
#include <stack>
#include <string>
#include <lexer.h>
#include <ast.h>

namespace mcf
{
	namespace Parser
	{
		enum class Precedence : unsigned char
		{
			INVALID = 0,

			LOWEST,
			EQUALS,			// == | !=
			LESSGREATER,	// > | <
			SUM,			// + | -
			PRODUCT,		// * | /
			PREFIX,			// -foo | !boo
			CALL,			// func(x)
			INDEX,			// foo[0]
			AS,				// "Hello, World!" as unsigned qword

			// 이 밑으로는 수정하면 안됩니다.
			COUNT
		};

		enum class ErrorID : unsigned char
		{
			INVALID = 0,

			SUCCESS,
			INVALID_LEXER_ERROR_TOKEN,
			INVALID_INPUT_LENGTH,
			FAIL_READ_FILE,
			FAIL_EXPRESSION_PARSING,
			FAIL_INTERMEDIATE_PARSING,
			FAIL_STATEMENT_PARSING,
			UNEXPECTED_NEXT_TOKEN,
			NOT_REGISTERED_EXPRESSION_TOKEN,
			NOT_REGISTERED_STATEMENT_TOKEN,

			// 이 밑으로는 수정하면 안됩니다.
			COUNT,
		};

		struct ErrorInfo final
		{
			mcf::Parser::ErrorID ID;
			std::string Name;
			std::string Message;
			size_t Line;
			size_t Index;
		};

		class Object final
		{
		public:
			explicit Object(void) noexcept = delete;
			explicit Object(const std::string& input, const bool isFile) noexcept;

			inline const size_t GetErrorCount(void) const noexcept { return _errors.size(); }
			const ErrorInfo PopLastError(void) noexcept;

			void ParseProgram(mcf::AST::Program& outProgram) noexcept;

		private:
			mcf::AST::Statement::Pointer ParseStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseIncludeLibraryStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseTypedefStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseExternStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseLetStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseBlockStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseReturnStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseFuncStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseMainStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseExpressionStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseUnusedStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseWhileStatement(void) noexcept;
			mcf::AST::Statement::Pointer ParseBreakStatement(void) noexcept;

			mcf::AST::Intermediate::Variadic::Pointer ParseVariadicIntermediate(void) noexcept;
			mcf::AST::Intermediate::TypeSignature::Pointer ParseTypeSignatureIntermediate(void) noexcept;
			mcf::AST::Intermediate::VariableSignature::Pointer ParseVariableSignatureIntermediate(void) noexcept;
			mcf::AST::Intermediate::FunctionParams::Pointer ParseFunctionParamsIntermediate(void) noexcept;
			mcf::AST::Intermediate::FunctionSignature::Pointer ParseFunctionSignatureIntermediate(void) noexcept;

			mcf::AST::Expression::Pointer ParseExpression(const Precedence precedence) noexcept;
			mcf::AST::Expression::Prefix::Pointer ParsePrefixExpression(void) noexcept;
			mcf::AST::Expression::Group::Pointer ParseGroupExpression(void) noexcept;
			mcf::AST::Expression::Infix::Pointer ParseInfixExpression(mcf::AST::Expression::Pointer&& left) noexcept;
			mcf::AST::Expression::Call::Pointer ParseCallExpression(mcf::AST::Expression::Pointer&& left) noexcept;
			mcf::AST::Expression::Index::Pointer ParseIndexExpression(mcf::AST::Expression::Pointer&& left) noexcept;
			mcf::AST::Expression::As::Pointer ParseAsExpression(mcf::AST::Expression::Pointer&& left) noexcept;
			mcf::AST::Expression::Initializer::Pointer ParseInitializerExpression(void) noexcept;
			mcf::AST::Expression::MapInitializer::Pointer ParseMapInitializerExpression(mcf::AST::Expression::PointerVector&& keyList) noexcept;

			inline void ReadNextToken(void) noexcept;
			inline const bool ReadNextTokenIf(const mcf::Token::Type tokenType) noexcept;
			inline const bool ReadNextTokenIfAny(std::initializer_list<mcf::Token::Type> list) noexcept;

			inline const bool IsCurrentTokenAny( std::initializer_list<mcf::Token::Type> list ) noexcept;

			inline const mcf::Parser::Precedence GetTokenPrecedence(const mcf::Token::Data& token) noexcept;
			inline const mcf::Parser::Precedence GetCurrentTokenPrecedence(void) noexcept;
			inline const mcf::Parser::Precedence GetNextTokenPrecedence(void) noexcept;

			inline const bool CheckErrorOnInit(void) noexcept;

		private:
			std::stack<ErrorInfo> _errors;
			mcf::Lexer::Object _lexer;
			mcf::Token::Data _currentToken;
			mcf::Token::Data _nextToken;
		};
	}
}