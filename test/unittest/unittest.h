#pragma once
#include <crtdbg.h>
#include <string>
#include <functional>

#include <common.h>
#include <lexer.h>
#include <parser.h>
#include <object.h>
#include <evaluator.h>
#include <compiler.h>

#if defined(_DEBUG)
#define FATAL_ASSERT(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); return false; } ((void)0)
#define FATAL_ERROR(FORMAT, ...) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); __debugbreak(); return false; } ((void)0)
#define ERROR_MESSAGE(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); } ((void)0)
#else
#define FATAL_ASSERT(PREDICATE, FORMAT, ...) if ((PREDICATE) == false) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); return false; } ((void)0)
#define FATAL_ERROR(FORMAT, ...) { printf("[Fatal Error]: %s(Line: %d)\n[Description]: ", ##__FILE__, ##__LINE__); printf(FORMAT, __VA_ARGS__); printf("\n"); return false; } ((void)0)
#endif

namespace UnitTest
{
	const mcf::Token::Data TokenInvalid = { mcf::Token::Type::INVALID, "invalid" };
	const mcf::Token::Data TokenEOF = { mcf::Token::Type::END_OF_FILE, "\0" };
	const mcf::Token::Data TokenByte = { mcf::Token::Type::IDENTIFIER, "byte" };
	const mcf::Token::Data TokenDword = { mcf::Token::Type::IDENTIFIER, "dword" };
	const mcf::Token::Data TokenQword = { mcf::Token::Type::IDENTIFIER, "qword" };
	inline const mcf::Token::Data TokenIdentifier(const char* const value) { return mcf::Token::Data{ mcf::Token::Type::IDENTIFIER, value }; }
	inline const mcf::Token::Data TokenInteger(const char* const value) { return mcf::Token::Data{ mcf::Token::Type::INTEGER, value }; }
	inline const mcf::Token::Data TokenString(const char* const value) { return mcf::Token::Data{ mcf::Token::Type::STRING, value }; }
	const mcf::Token::Data TokenAssign = { mcf::Token::Type::ASSIGN, "=" };
	const mcf::Token::Data TokenPlus = { mcf::Token::Type::PLUS, "+" };
	const mcf::Token::Data TokenMinus = { mcf::Token::Type::MINUS, "-" };
	const mcf::Token::Data TokenAsterisk = { mcf::Token::Type::ASTERISK, "*" };
	const mcf::Token::Data TokenSlash = { mcf::Token::Type::SLASH, "/" };
	const mcf::Token::Data TokenLT = { mcf::Token::Type::LT, "<" };
	const mcf::Token::Data TokenGT = { mcf::Token::Type::GT, ">" };
	const mcf::Token::Data TokenAmpersand = { mcf::Token::Type::AMPERSAND, "&" };
	const mcf::Token::Data TokenLParen = { mcf::Token::Type::LPAREN, "(" };
	const mcf::Token::Data TokenRParen = { mcf::Token::Type::RPAREN, ")" };
	const mcf::Token::Data TokenLBrace = { mcf::Token::Type::LBRACE, "{" };
	const mcf::Token::Data TokenRBrace = { mcf::Token::Type::RBRACE, "}" };
	const mcf::Token::Data TokenLBracket = { mcf::Token::Type::LBRACKET, "[" };
	const mcf::Token::Data TokenRBracket = { mcf::Token::Type::RBRACKET, "]" };
	const mcf::Token::Data TokenColon = { mcf::Token::Type::COLON, ":" };
	const mcf::Token::Data TokenSemicolon = { mcf::Token::Type::SEMICOLON, ";" };
	const mcf::Token::Data TokenComma = { mcf::Token::Type::COMMA, "," };
	const mcf::Token::Data TokenPointing = { mcf::Token::Type::POINTING, "->" };
	const mcf::Token::Data TokenASM = { mcf::Token::Type::KEYWORD_ASM, "asm" };
	const mcf::Token::Data TokenExtern = { mcf::Token::Type::KEYWORD_EXTERN, "extern" };
	const mcf::Token::Data TokenTypedef = { mcf::Token::Type::KEYWORD_TYPEDEF, "typedef" };
	const mcf::Token::Data TokenLet = { mcf::Token::Type::KEYWORD_LET, "let" };
	const mcf::Token::Data TokenFunc = { mcf::Token::Type::KEYWORD_FUNC, "func" };
	const mcf::Token::Data TokenMain = { mcf::Token::Type::KEYWORD_MAIN, "main" };
	const mcf::Token::Data TokenVoid = { mcf::Token::Type::KEYWORD_VOID, "void" };
	const mcf::Token::Data TokenUnsigned = { mcf::Token::Type::KEYWORD_UNSIGNED, "unsigned" };
	const mcf::Token::Data TokenReturn = { mcf::Token::Type::KEYWORD_RETURN, "return" };
	const mcf::Token::Data TokenUnused = { mcf::Token::Type::KEYWORD_UNUSED, "unused" };
	const mcf::Token::Data TokenAs = { mcf::Token::Type::KEYWORD_AS, "as" };
	const mcf::Token::Data TokenVariadic = { mcf::Token::Type::VARIADIC, "..." };
	const mcf::Token::Data TokenInclude = { mcf::Token::Type::MACRO_INCLUDE, "#include" };
	inline const mcf::Token::Data TokenComment(const char* const value) { return mcf::Token::Data{ mcf::Token::Type::COMMENT, value }; }
	inline const mcf::Token::Data TokenCommentBlock(const char* const value) { return mcf::Token::Data{ mcf::Token::Type::COMMENT_BLOCK, value }; }

	class BaseTest
	{
	public:
		virtual inline ~BaseTest(void) noexcept {}
		const bool Test(void) const noexcept;

	protected:
		std::vector<std::string>			_names;
		std::vector<std::function<bool()>>	_tests;
	};

	class LexerTest final : public BaseTest
	{
	public:
		explicit LexerTest(void) noexcept;
	};

	class ParserTest final : public BaseTest
	{
	public:
		explicit ParserTest(void) noexcept;

	private:
		static bool CheckParserErrors(mcf::Parser::Object& parser) noexcept;
	};

	class EvaluatorTest final : public BaseTest
	{
	public:
		explicit EvaluatorTest(void) noexcept;

	private:
		static bool CheckParserErrors(mcf::Parser::Object& parser) noexcept;
	};

	class CompilerTest final : public BaseTest
	{
	public:
		explicit CompilerTest(void) noexcept;

	private:
		static bool CheckParserErrors(mcf::Parser::Object& parser) noexcept;
	};

	inline static void DetectMemoryLeak( long line = -1 )
	{
		//Also need this for memory leak code stuff
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetBreakAlloc(line); //Important!
	}

	inline static bool InternalTest(const char* const name, const UnitTest::BaseTest* const test)
	{
		std::cout << "`" << name << ".Test()` Begin" << std::endl;
		if (test->Test() == false)
		{
			std::cout << "\t`" << name << ".Test()` Failed" << std::endl;
			return false;
		}
		std::cout << "\t`" << name << ".Test()` Passed" << std::endl;
		std::cout << "`" << name << ".Test()` End" << std::endl;
		return true;
	}
}
