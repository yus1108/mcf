#include "pch.h"
#include "parser/internals/internal_parser.h"

mcf::Parser::Object::Object(const std::string& input, const bool isFile) noexcept
	: _lexer(input, isFile)
{
	if (CheckErrorOnInit() == false)
	{
		return;
	}
	ReadNextToken(); // _currentToken = invalid; _nextToken = valid;
	ReadNextToken(); // _currentToken = valid; _nextToken = valid;
}

const mcf::Parser::ErrorInfo mcf::Parser::Object::PopLastError(void) noexcept
{
	if (_errors.empty())
	{
		return { ErrorID::SUCCESS, _lexer.GetName(), std::string(), 0, 0 };
	}

	const ErrorInfo error = _errors.top();
	_errors.pop();
	return error;
}

void mcf::Parser::Object::ParseProgram(mcf::AST::Program& outProgram) noexcept
{
	mcf::AST::Statement::PointerVector statements;
	while (_currentToken.Type != mcf::Token::Type::END_OF_FILE)
	{
		statements.emplace_back(ParseStatement());
		if (statements.back().get() == nullptr)
		{
			const std::string message = ErrorMessage(u8"파싱에 실패하였습니다.");
			_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return;
		}

		// read next token
		ReadNextToken();
	}
	outProgram = mcf::AST::Program(std::move(statements));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseStatement(void) noexcept
{
	mcf::AST::Statement::Pointer statement;
	constexpr const size_t TOKENTYPE_COUNT_BEGIN = __COUNTER__;
	switch (_currentToken.Type)
	{
	case Token::Type::MACRO_INCLUDE: __COUNTER__;
		statement = ParseIncludeLibraryStatement();
		break;

	case Token::Type::KEYWORD_TYPEDEF: __COUNTER__;
		statement = ParseTypedefStatement();
		break;

	case Token::Type::KEYWORD_EXTERN: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case Token::Type::KEYWORD_LET: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case Token::Type::KEYWORD_FUNC: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case Token::Type::KEYWORD_RETURN: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case Token::Type::KEYWORD_MAIN: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
	case Token::Type::SEMICOLON: __COUNTER__;
		break;

	case Token::Type::IDENTIFIER: __COUNTER__; [[fallthrough]];
	case Token::Type::INTEGER: __COUNTER__; [[fallthrough]];
	case Token::Type::STRING: __COUNTER__; [[fallthrough]];
	case Token::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	case Token::Type::PLUS: __COUNTER__; [[fallthrough]];
	case Token::Type::MINUS: __COUNTER__; [[fallthrough]];
	case Token::Type::ASTERISK: __COUNTER__; [[fallthrough]];
	case Token::Type::SLASH: __COUNTER__; [[fallthrough]];
	case Token::Type::BANG: __COUNTER__; [[fallthrough]];
	case Token::Type::EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::NOT_EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::LT: __COUNTER__; [[fallthrough]];
	case Token::Type::GT: __COUNTER__; [[fallthrough]];
	case Token::Type::AMPERSAND: __COUNTER__; [[fallthrough]];
	case Token::Type::LPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::RPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::DOUBLE_COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMA: __COUNTER__; [[fallthrough]];
	case Token::Type::POINTING: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_START: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_ASM: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_BIND: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNUSED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_END: __COUNTER__; [[fallthrough]];
	case Token::Type::VARIADIC: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_START: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_END: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMENT: __COUNTER__; [[fallthrough]];			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case Token::Type::COMMENT_BLOCK: __COUNTER__; [[fallthrough]];	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
	{
		const std::string message = ErrorMessage(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. TokenType=%s(%zu) TokenLiteral=`%s`",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());
		_errors.push(ErrorInfo{ ErrorID::NOT_REGISTERED_STATEMENT_TOKEN, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		break;
	}
	}
	constexpr const size_t TOKENTYPE_COUNT = __COUNTER__ - TOKENTYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == TOKENTYPE_COUNT, "TokenType count is changed. this SWITCH need to be changed as well.");
	return statement;
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseIncludeLibraryStatement(void) noexcept
{
	DebugAssert(_currentToken.Type == mcf::Token::Type::MACRO_INCLUDE, u8"이 함수가 호출될때 현재 토큰이 `MACRO_INCLUDE`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	if (ReadNextTokenIf(mcf::Token::Type::LT) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `LT`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Pointer();
	}

	if (ReadNextTokenIf(mcf::Token::Type::KEYWORD_ASM) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `KEYWORD_ASM`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Pointer();
	}

	if (ReadNextTokenIf(mcf::Token::Type::COMMA) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `COMMA`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Pointer();
	}

	if (ReadNextTokenIf(mcf::Token::Type::STRING) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `STRING`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Pointer();
	}
	mcf::Token::Data libPath = _currentToken;

	if (ReadNextTokenIf(mcf::Token::Type::GT) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `GT`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Pointer();
	}
	return mcf::AST::Statement::IncludeLibrary::Make(libPath);
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseTypedefStatement(void) noexcept
{
	DebugAssert(_currentToken.Type == mcf::Token::Type::KEYWORD_TYPEDEF, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_TYPEDEF`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	ReadNextToken();
	mcf::AST::Statement::Typedef::SignaturePointer signature = ParseVariableSignature();
	if (signature.get() == nullptr)
	{
		const std::string message = ErrorMessage(u8"Typedef 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	mcf::AST::Statement::Typedef::BindMapPointer bindMap;
	if (ReadNextTokenIf(mcf::Token::Type::POINTING) == true)
	{
		if (ReadNextTokenIf(mcf::Token::Type::KEYWORD_BIND) == false)
		{
			const std::string message = ErrorMessage(u8"다음 토큰은 `KEYWORD_BIND`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return mcf::AST::Statement::Invalid::Pointer();
		}

		ReadNextToken();
		bindMap = ParseMapInitializerIntermeidate();
		if (bindMap.get() == nullptr)
		{
			const std::string message = ErrorMessage(u8"Typedef 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}
	}

	if (ReadNextTokenIf(mcf::Token::Type::SEMICOLON) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `SEMICOLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Pointer();
	}

	return mcf::AST::Statement::Typedef::Make(std::move(signature), std::move(bindMap));
}

mcf::AST::Intermediate::MapInitializer::Pointer mcf::Parser::Object::ParseMapInitializerIntermeidate(void) noexcept
{
	DebugAssert(_currentToken.Type == mcf::Token::Type::LBRACE, u8"이 함수가 호출될때 현재 토큰이 `LBRACE`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	std::vector<mcf::AST::Intermediate::MapInitializer::KeyValue> itemList;
	mcf::AST::Intermediate::MapInitializer::KeyValue firstMapItem;

	ReadNextToken();
	firstMapItem.first = ParseExpression(Precedence::LOWEST);
	if (firstMapItem.first.get() == nullptr || firstMapItem.first->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = ErrorMessage(u8"MapInitializer key 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::ASSIGN) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `ASSIGN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	ReadNextToken();
	firstMapItem.second = ParseExpression(Precedence::LOWEST);
	if (firstMapItem.second.get() == nullptr || firstMapItem.second->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = ErrorMessage(u8"MapInitializer value 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}
	itemList.emplace_back(std::move(firstMapItem));

	if (ReadNextTokenIf(mcf::Token::Type::RBRACE) == true)
	{
		return mcf::AST::Intermediate::MapInitializer::Make(std::move(itemList));
	}

	while (ReadNextTokenIf(mcf::Token::Type::COMMA) == true)
	{
		if (ReadNextTokenIf(mcf::Token::Type::RBRACE) == true)
		{
			return mcf::AST::Intermediate::MapInitializer::Make(std::move(itemList));
		}

		mcf::AST::Intermediate::MapInitializer::KeyValue nextMapItem;

		ReadNextToken();
		nextMapItem.first = ParseExpression(Precedence::LOWEST);
		if (nextMapItem.first.get() == nullptr || nextMapItem.first->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
		{
			const std::string message = ErrorMessage(u8"MapInitializer key 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}

		if (ReadNextTokenIf(mcf::Token::Type::ASSIGN) == false)
		{
			const std::string message = ErrorMessage(u8"다음 토큰은 `ASSIGN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		ReadNextToken();
		nextMapItem.second = ParseExpression(Precedence::LOWEST);
		if (nextMapItem.second.get() == nullptr || nextMapItem.second->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
		{
			const std::string message = ErrorMessage(u8"MapInitializer value 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}
		itemList.emplace_back(std::move(nextMapItem));
	}

	if (ReadNextTokenIf(mcf::Token::Type::RBRACE) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `RBRACE`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Intermediate::MapInitializer::Make(std::move(itemList));
}

mcf::AST::Intermediate::TypeSignature::Pointer mcf::Parser::Object::ParseTypeSignatureIntermediate(void) noexcept
{
	DebugAssert(_currentToken.Type == mcf::Token::Type::IDENTIFIER, u8"이 함수가 호출될때 현재 토큰이 `IDENTIFIER`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	mcf::AST::Expression::Pointer signature = mcf::AST::Expression::Identifier::Make(_currentToken);
	while (ReadNextTokenIf(mcf::Token::Type::LBRACKET) == true)
	{
		signature = ParseIndexExpression(std::move(signature));
		if (signature.get() == nullptr)
		{
			const std::string message = ErrorMessage(u8"TypeSignature 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}
	}
	return mcf::AST::Intermediate::TypeSignature::Make(std::move(signature));
}

mcf::AST::Intermediate::VariableSignature::Pointer mcf::Parser::Object::ParseVariableSignature(void) noexcept
{
	DebugAssert(_currentToken.Type == mcf::Token::Type::IDENTIFIER, u8"이 함수가 호출될때 현재 토큰이 `IDENTIFIER`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	mcf::AST::Expression::Identifier::Pointer name = mcf::AST::Expression::Identifier::Make(_currentToken);
	if (ReadNextTokenIf(mcf::Token::Type::COLON) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `COLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	ReadNextToken();
	mcf::AST::Intermediate::TypeSignature::Pointer typeSignature = ParseTypeSignatureIntermediate();
	if (typeSignature.get() == nullptr)
	{
		const std::string message = ErrorMessage(u8"VariableSignature 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	return mcf::AST::Intermediate::VariableSignature::Make(std::move(name), std::move(typeSignature));
}

mcf::AST::Expression::Pointer mcf::Parser::Object::ParseExpression(const Precedence precedence) noexcept
{
	mcf::AST::Expression::Pointer expression;
	constexpr const size_t EXPRESSION_TOKEN_COUNT_BEGIN = __COUNTER__;
	switch (_currentToken.Type)
	{
	case Token::Type::IDENTIFIER: __COUNTER__;
		expression = mcf::AST::Expression::Identifier::Make(_currentToken);
		break;

	case Token::Type::INTEGER: __COUNTER__;
		expression = mcf::AST::Expression::Integer::Make(_currentToken);
		break;

	case Token::Type::STRING: __COUNTER__;
		expression = mcf::AST::Expression::String::Make(_currentToken);
		break;

	case Token::Type::AMPERSAND: __COUNTER__;
		DebugMessage(u8"구현 필요");
		break;

	case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
	case Token::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	case Token::Type::PLUS: __COUNTER__; [[fallthrough]];
	case Token::Type::MINUS: __COUNTER__; [[fallthrough]];
	case Token::Type::ASTERISK: __COUNTER__; [[fallthrough]];
	case Token::Type::SLASH: __COUNTER__; [[fallthrough]];
	case Token::Type::BANG: __COUNTER__; [[fallthrough]];
	case Token::Type::EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::NOT_EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::LT: __COUNTER__; [[fallthrough]];
	case Token::Type::GT: __COUNTER__; [[fallthrough]];
	case Token::Type::LPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::RPAREN: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::DOUBLE_COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::SEMICOLON: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMA: __COUNTER__; [[fallthrough]];
	case Token::Type::POINTING: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_START: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_ASM: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_EXTERN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_TYPEDEF: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_BIND: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_LET: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_FUNC: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_MAIN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_RETURN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNUSED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_END: __COUNTER__; [[fallthrough]];
	case Token::Type::VARIADIC: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_START: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_INCLUDE: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_END: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMENT: __COUNTER__; [[fallthrough]];			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case Token::Type::COMMENT_BLOCK: __COUNTER__; [[fallthrough]];	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
	{
		const std::string message = ErrorMessage(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. TokenType=%s(%zu) TokenLiteral=`%s`",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());
		_errors.push(ErrorInfo{ ErrorID::NOT_REGISTERED_EXPRESSION_TOKEN, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		break;
	}
	}
	constexpr const size_t EXPRESSION_TOKEN_COUNT = __COUNTER__ - EXPRESSION_TOKEN_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == EXPRESSION_TOKEN_COUNT, "TokenType count is changed. this SWITCH need to be changed as well.");

	while (expression.get() != nullptr && _nextToken.Type != mcf::Token::Type::SEMICOLON && precedence < GetNextTokenPrecedence())
	{
		constexpr const size_t INFIX_COUNT_BEGIN = __COUNTER__;
		switch (_nextToken.Type)
		{
		case Token::Type::PLUS: __COUNTER__; [[fallthrough]];
		case Token::Type::MINUS: __COUNTER__; [[fallthrough]];
		case Token::Type::ASTERISK: __COUNTER__; [[fallthrough]];
		case Token::Type::SLASH: __COUNTER__; [[fallthrough]];
		case Token::Type::EQUAL: __COUNTER__; [[fallthrough]];
		case Token::Type::NOT_EQUAL: __COUNTER__;
			ReadNextToken();
			DebugMessage(u8"구현 필요");
			break;

		case Token::Type::LPAREN: __COUNTER__;
			ReadNextToken();
			DebugMessage(u8"구현 필요");
			break;

		case Token::Type::LBRACKET: __COUNTER__;
			ReadNextToken();
			expression = ParseIndexExpression(std::move(expression));
			break;

		case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
		case Token::Type::IDENTIFIER: __COUNTER__; [[fallthrough]];
		case Token::Type::INTEGER: __COUNTER__; [[fallthrough]];
		case Token::Type::STRING: __COUNTER__; [[fallthrough]];
		case Token::Type::ASSIGN: __COUNTER__; [[fallthrough]];
		case Token::Type::BANG: __COUNTER__; [[fallthrough]];
		case Token::Type::LT: __COUNTER__; [[fallthrough]];
		case Token::Type::GT: __COUNTER__; [[fallthrough]];
		case Token::Type::AMPERSAND: __COUNTER__;
		case Token::Type::RPAREN: __COUNTER__; [[fallthrough]];
		case Token::Type::LBRACE: __COUNTER__; [[fallthrough]];
		case Token::Type::RBRACE: __COUNTER__; [[fallthrough]];
		case Token::Type::RBRACKET: __COUNTER__; [[fallthrough]];
		case Token::Type::COLON: __COUNTER__; [[fallthrough]];
		case Token::Type::DOUBLE_COLON: __COUNTER__; [[fallthrough]];
		case Token::Type::SEMICOLON: __COUNTER__; [[fallthrough]];
		case Token::Type::COMMA: __COUNTER__; [[fallthrough]];
		case Token::Type::POINTING: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_IDENTIFIER_START: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_ASM: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_EXTERN: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_TYPEDEF: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_BIND: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_LET: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_FUNC: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_MAIN: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_RETURN: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_UNUSED: __COUNTER__; [[fallthrough]];
		case Token::Type::KEYWORD_IDENTIFIER_END: __COUNTER__; [[fallthrough]];
		case Token::Type::VARIADIC: __COUNTER__; [[fallthrough]];
		case Token::Type::MACRO_START: __COUNTER__; [[fallthrough]];
		case Token::Type::MACRO_INCLUDE: __COUNTER__; [[fallthrough]];
		case Token::Type::MACRO_END: __COUNTER__; [[fallthrough]];
		case Token::Type::COMMENT: __COUNTER__; [[fallthrough]];			// 주석은 파서에서 토큰을 읽으면 안됩니다.
		case Token::Type::COMMENT_BLOCK: __COUNTER__; [[fallthrough]];	// 주석은 파서에서 토큰을 읽으면 안됩니다.
		default:
		{
			const std::string message = ErrorMessage(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. TokenType=%s(%zu) TokenLiteral=`%s`",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());
			_errors.push(ErrorInfo{ ErrorID::NOT_REGISTERED_EXPRESSION_TOKEN, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			break;
		}
		}
		constexpr const size_t INFIX_COUNT = __COUNTER__ - INFIX_COUNT_BEGIN;
		static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == INFIX_COUNT, "TokenType count is changed. this SWITCH need to be changed as well.");
	}
	return expression.get() == nullptr ? mcf::AST::Expression::Invalid::Pointer() : std::move(expression);
}

mcf::AST::Expression::Index::Pointer mcf::Parser::Object::ParseIndexExpression(mcf::AST::Expression::Pointer&& left) noexcept
{
	DebugAssert(_currentToken.Type == mcf::Token::Type::LBRACKET, u8"이 함수가 호출될때 현재 토큰이 `LBRACKET`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	if (ReadNextTokenIf(mcf::Token::Type::RBRACKET) == true)
	{
		return mcf::AST::Expression::Index::Make(std::move(left), nullptr);
	}

	ReadNextToken();
	mcf::AST::Expression::Pointer index = ParseExpression(Precedence::LOWEST);
	if (index.get() == nullptr || index->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = ErrorMessage(u8"Index 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::RBRACKET) == false)
	{
		const std::string message = ErrorMessage(u8"다음 토큰은 `RBRACE`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Expression::Index::Make(std::move(left), std::move(index));
}
