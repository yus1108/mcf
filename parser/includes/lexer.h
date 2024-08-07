#pragma once
#include <stack>
#include <string>
#include <parser/includes/common.h>

namespace mcf
{
	namespace Token
	{
		enum class Type : unsigned char
		{
			INVALID = 0,
			END_OF_FILE, // \0

			// 식별자 + 리터럴
			IDENTIFIER,				// [_a-zA-Z]+[_a-zA-Z0-9]*
			INTEGER,				// [0-9]+
			STRING_UTF8,			// "[^"\n\r]*"

			// 연산자
			ASSIGN,		// =
			PLUS,		// +
			MINUS,		// -
			ASTERISK,	// *
			SLASH,		// /
			BANG,		// !
			EQUAL,		// ==
			NOT_EQUAL,	// !=
			LT,			// <
			GT,			// >
			AMPERSAND,	// &

			LPAREN,		// (
			RPAREN,		// )
			LBRACE,		// {
			RBRACE,		// }
			LBRACKET,	// [
			RBRACKET,	// ]

			// 구분자
			COLON,			// :
			DOUBLE_COLON,	// ::
			SEMICOLON,		// ;
			COMMA,			// ,

			// 식별자 키워드
			KEYWORD_IDENTIFIER_START,	// 실제 값으로 사용되어선 안됩니다!!!
			KEYWORD_CONST,				// const
			KEYWORD_VOID,				// void
			KEYWORD_INT8,				// int8
			KEYWORD_INT16,				// int16
			KEYWORD_INT32,				// int32
			KEYWORD_INT64,				// int64
			KEYWORD_UINT8,				// uint8
			KEYWORD_UINT16,				// uint16
			KEYWORD_UINT32,				// uint32
			KEYWORD_UINT64,				// uint64
			KEYWORD_UTF8,				// utf8
			KEYWORD_ENUM,				// enum
			KEYWORD_UNUSED,				// unused
			KEYWORD_IN,					// in
			KEYWORD_OUT,				// out
			KEYWORD_BOOL,				// bool
			KEYWORD_TRUE,				// true
			KEYWORD_FALSE,				// false
			KEYWORD_IDENTIFIER_END,		// 실제 값으로 사용되어선 안됩니다!!!

			CUSTOM_KEYWORD_START,	// 실제 값으로 사용되어선 안됩니다!!!
			CUSTOM_ENUM_TYPE,		// 커스텀 열거형 타입
			CUSTOM_KEYWORD_END,		// 실제 값으로 사용되어선 안됩니다!!!

			// '.' 으로 시작하는 토큰
			KEYWORD_VARIADIC,	// ...

			// 매크로
			MACRO_START,				// 실제 값으로 사용되어선 안됩니다!!!
			MACRO_IIBRARY_FILE_INCLUDE,	// #include <[^<>\N\R]+>
			MACRO_PROJECT_FILE_INCLUDE,	// #include "[^<>\N\R]+"
			MACRO_END,					// 실제 값으로 사용되어선 안됩니다!!!

			// 주석
			COMMENT,		// //[^\n\r]
			COMMENT_BLOCK,	// /*[^"*/"]*/

			// 이 밑으로는 수정하면 안됩니다.
			COUNT
		};

		constexpr const char* TYPE_STRING_ARRAY[] =
		{
			"INVALID",
			"END_OF_FILE",

			// 식별자 + 리터럴
			"IDENTIFIER",
			"INTEGER",
			"STRING_UTF8",

			// 연산자
			"ASSIGN",
			"PLUS",
			"MINUS",
			"ASTERISK",
			"SLASH",
			"BANG",
			"EQUAL",
			"NOT_EQUAL",
			"LT",
			"GT",
			"AMPERSAND",

			"LPAREN",
			"RPAREN",
			"LBRACE",
			"RBRACE",
			"LBRACKET",
			"RBRACKET",


			// 구분자
			"COLON",
			"DOUBLE_COLON",
			"SEMICOLON",
			"COMMA",

			// 식별자 키워드
			"KEYWORD_IDENTIFIER_START",
			"KEYWORD_CONST",
			"KEYWORD_VOID",
			"KEYWORD_INT8",
			"KEYWORD_INT16",
			"KEYWORD_INT32",
			"KEYWORD_INT64",
			"KEYWORD_UINT8",
			"KEYWORD_UINT16",
			"KEYWORD_UINT32",
			"KEYWORD_UINT64",
			"KEYWORD_UTF8",
			"KEYWORD_ENUM",
			"KEYWORD_UNUSED",
			"KEYWORD_IN",
			"KEYWORD_OUT",
			"KEYWORD_BOOL",
			"KEYWORD_TRUE",
			"KEYWORD_FALSE",
			"KEYWORD_IDENTIFIER_END",

			"CUSTOM_KEYWORD_START",
			"CUSTOM_ENUM_TYPE",
			"CUSTOM_KEYWORD_END",

			// '.' 으로 시작하는 토큰
			"KEYWORD_VARIADIC",

			// 매크로
			"MACRO_START",
			"MACRO_IIBRARY_FILE_INCLUDE",
			"MACRO_PROJECT_FILE_INCLUDE",
			"MACRO_END",

			"COMMENT",
			"COMMENT_BLOCK",
		};
		constexpr const size_t TOKEN_TYPES_SIZE = ARRAY_SIZE(TYPE_STRING_ARRAY);
		static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == TOKEN_TYPES_SIZE, "token count not matching!");

		constexpr const char* CONVERT_TYPE_TO_STRING(const mcf::Token::Type& value)
		{
			return mcf::Token::TYPE_STRING_ARRAY[mcf::ENUM_INDEX(value)];
		}

		struct Data final
		{
			mcf::Token::Type Type = mcf::Token::Type::INVALID;
			std::string Literal;
			size_t Line = 0;
			size_t Index = 0;
		};
		inline bool operator==(const mcf::Token::Data& lhs, const mcf::Token::Data& rhs) { return (lhs.Type == rhs.Type) && (lhs.Literal == rhs.Literal); }
		static const mcf::Token::Data FindPredefinedKeyword(const std::string& tokenLiteral) noexcept;
	}

	namespace Lexer
	{
		enum class Error : unsigned char
		{
			INVALID = 0,

			SUCCESS,
			INVALID_INPUT_LENGTH,
			FAIL_READ_FILE,
			FAIL_MEMORY_ALLOCATION,
			REGISTERING_DUPLICATED_SYMBOL_NAME,

			COUNT,
		};

		// 주의: thread-safe 하지 않은 클래스입니다.
		class Object final 
		{
		public:
			explicit Object(void) noexcept = delete;
			explicit Object(const std::string& input, const bool isFIle) noexcept;

			const mcf::Lexer::Error GetLastErrorToken(void) noexcept;
			const std::string GetName(void) const noexcept { return _name; }

			const mcf::Token::Data ReadNextToken(void) noexcept;

		private:
			inline const char GetNextByte(void) const noexcept;

			inline void ReadNextByte(void) noexcept;
			inline const bool ReadLineIfStartWith(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* startWith) noexcept;
			inline const bool ReadAndValidate(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* stringToCompare) noexcept;
			inline const bool ReadAndValidate(_Outptr_opt_ std::string* optionalOut, _In_opt_ const char* startWith, _In_opt_ const char* endWith, _In_opt_ const char* invalidCharList) noexcept;
			inline const std::string ReadKeywordOrIdentifier(void) noexcept;
			inline const std::string ReadNumber(void) noexcept;
			inline const mcf::Token::Data ReadStringUtf8(void) noexcept;
			inline const mcf::Token::Data ReadSlashStartingToken(void) noexcept;
			inline const mcf::Token::Data ReadDotStartingToken(void) noexcept;
			inline const mcf::Token::Data ReadMacroToken(void) noexcept;
			inline const mcf::Token::Data ReadNumeric(void) noexcept;

			inline const mcf::Token::Type DetermineKeywordOrIdentifier(const std::string& tokenLiteral) noexcept;

		private:
			std::stack<mcf::Lexer::Error> _tokens;
			const std::string _input;
			const std::string _name;
			size_t _currentPosition = 0;
			size_t _nextPosition = 0;
			size_t _currentLine = 1; // 코드 명령줄은 항상 1부터 시작합니다.
			size_t _currentIndex = 0;
			char _currentByte = 0;
		};
	}
}