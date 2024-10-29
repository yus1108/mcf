#pragma once
#include "framework.h"
#include "parser.h"

inline void mcf::Parser::Object::ReadNextToken(void) noexcept
{
	_currentToken = _nextToken;
	_nextToken = _lexer.ReadNextToken();
	// 읽은 토큰이 주석이라면 주석이 아닐때까지 읽는다.
	while (_nextToken.Type == Token::Type::COMMENT || _nextToken.Type == Token::Type::COMMENT_BLOCK)
	{
		_nextToken = _lexer.ReadNextToken();
	}
}

inline const bool mcf::Parser::Object::ReadNextTokenIf(const mcf::Token::Type tokenType) noexcept
{
	if (_nextToken.Type != tokenType)
	{
		return false;
	}
	ReadNextToken();
	return true;
}

inline const bool mcf::Parser::Object::ReadNextTokenIfAny(std::initializer_list<mcf::Token::Type> list) noexcept
{
	for (const mcf::Token::Type* iter = list.begin(); iter != list.end(); iter++)
	{
		if ( _nextToken.Type == *iter)
		{
			ReadNextToken();
			return true;
		}
	}
	return false;
}

inline const bool mcf::Parser::Object::IsCurrentTokenAny(std::initializer_list<mcf::Token::Type> list) noexcept
{
	for ( const mcf::Token::Type* iter = list.begin(); iter != list.end(); iter++ )
	{
		if ( _currentToken.Type == *iter )
		{
			return true;
		}
	}
	return false;
}

inline const mcf::Parser::Precedence mcf::Parser::Object::GetTokenPrecedence(const mcf::Token::Data& token) noexcept
{
	constexpr const size_t PRECEDENCE_COUNT_BEGIN = __COUNTER__;
	switch (token.Type)
	{
	case Token::Type::PLUS: __COUNTER__; [[fallthrough]];
	case Token::Type::MINUS: __COUNTER__;
		return Precedence::SUM;

	case Token::Type::ASTERISK: __COUNTER__; [[fallthrough]];
	case Token::Type::SLASH: __COUNTER__;
		return Precedence::PRODUCT;

	case Token::Type::EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::NOT_EQUAL: __COUNTER__;
		return Precedence::EQUALS;

	case Token::Type::LT: __COUNTER__; [[fallthrough]];
	case Token::Type::GT: __COUNTER__;
		return Precedence::LESSGREATER;

	case Token::Type::LPAREN: __COUNTER__;
		return Precedence::CALL;

	case Token::Type::LBRACKET: __COUNTER__;
		return Precedence::INDEX;

	case Token::Type::KEYWORD_AS: __COUNTER__;
		return Precedence::AS;

	case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
	case Token::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	case Token::Type::RPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::SEMICOLON: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMA: __COUNTER__;
		break;

	case Token::Type::IDENTIFIER: __COUNTER__; [[fallthrough]];
	case Token::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case Token::Type::STRING: __COUNTER__; [[fallthrough]];
	case Token::Type::BANG: __COUNTER__; [[fallthrough]];
	case Token::Type::AMPERSAND: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::DOUBLE_COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::POINTING: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_START: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_ASM: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_EXTERN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_TYPEDEF: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_LET: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_FUNC: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_MAIN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_VOID: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNSIGNED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_RETURN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNUSED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_WHILE: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_BREAK: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_END: __COUNTER__; [[fallthrough]];
	case Token::Type::VARIADIC: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_START: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_INCLUDE: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_END: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMENT: __COUNTER__; [[fallthrough]];			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case Token::Type::COMMENT_BLOCK: __COUNTER__; [[fallthrough]];	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
		MCF_DEBUG_BREAK(u8"예상치 못한 값이 들어왔습니다. 에러가 아닐 수도 있습니다. 확인 해 주세요. TokenType=%s(%zu) TokenLiteral=`%s`",
			mcf::Token::CONVERT_TYPE_TO_STRING(token.Type), mcf::ENUM_INDEX(token.Type), token.Literal.c_str());
		break;
	}
	constexpr const size_t PRECEDENCE_COUNT = __COUNTER__ - PRECEDENCE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == PRECEDENCE_COUNT, "get token precedence for infix.");

	return Precedence::LOWEST;
}

inline const mcf::Parser::Precedence mcf::Parser::Object::GetCurrentTokenPrecedence(void) noexcept
{
	return GetTokenPrecedence(_currentToken);
}

inline const mcf::Parser::Precedence mcf::Parser::Object::GetNextTokenPrecedence(void) noexcept
{
	return GetTokenPrecedence(_nextToken);
}

inline const bool mcf::Parser::Object::CheckErrorOnInit(void) noexcept
{
	// 렉서에 에러가 있는지 체크
	mcf::Lexer::Error lexerError = _lexer.GetLastErrorToken();
	constexpr const size_t LEXER_ERROR_TOKEN_COUNT_BEGIN = __COUNTER__;
	switch (lexerError)
	{
	case mcf::Lexer::Error::SUCCESS: __COUNTER__;
		return true;

	case mcf::Lexer::Error::INVALID_INPUT_LENGTH: __COUNTER__;
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"input의 길이가 0입니다.");
		_errors.push(ErrorInfo{ ErrorID::INVALID_INPUT_LENGTH, _lexer.GetName(), message, 0, 0 });
		break;
	}

	case mcf::Lexer::Error::FAIL_READ_FILE: __COUNTER__;
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"파일 읽기에 실패 하였습니다. file path=%s", _lexer.GetName().c_str());
		_errors.push(ErrorInfo{ ErrorID::FAIL_READ_FILE, _lexer.GetName(), message, 0, 0 });
		break;
	}

	default:
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. ErrorID=INVALID_LEXER_ERROR_TOKEN");
		_errors.push(ErrorInfo{ ErrorID::INVALID_LEXER_ERROR_TOKEN, _lexer.GetName(), message, 0, 0 });
		break;
	}
	}
	constexpr const size_t LEXER_ERROR_TOKEN_COUNT = __COUNTER__ - LEXER_ERROR_TOKEN_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Lexer::Error::COUNT) == LEXER_ERROR_TOKEN_COUNT, "Lexer::Error is changed. this SWITCH need to be changed as well.");

	return false;
}