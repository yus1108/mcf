﻿#include "pch.h"
#include "internal_parser.h"

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
		if (statements.back().get() == nullptr || statements.back()->GetStatementType() == mcf::AST::Statement::Type::INVALID)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"파싱에 실패하였습니다.");
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
		statement = ParseExternStatement();
		break;

	case Token::Type::KEYWORD_LET: __COUNTER__;
		statement = ParseLetStatement();
		break;

	case Token::Type::LBRACE: __COUNTER__;
		statement = ParseBlockStatement();
		break;

	case Token::Type::KEYWORD_RETURN: __COUNTER__;
		statement = ParseReturnStatement();
		break;

	case Token::Type::KEYWORD_FUNC: __COUNTER__;
		statement = ParseFuncStatement();
		break;

	case Token::Type::KEYWORD_MAIN: __COUNTER__;
		statement = ParseMainStatement();
		break;

	case Token::Type::IDENTIFIER: __COUNTER__;
		statement = ParseExpressionStatement();
		break;

	case Token::Type::KEYWORD_UNUSED: __COUNTER__;
		statement = ParseUnusedStatement();
		break;

	case Token::Type::KEYWORD_WHILE: __COUNTER__;
		statement = ParseWhileStatement();
		break;

	case Token::Type::KEYWORD_BREAK: __COUNTER__;
		statement = ParseBreakStatement();
		break;

	case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
	case Token::Type::SEMICOLON: __COUNTER__;
		break;

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
	case Token::Type::RBRACE: __COUNTER__; [[fallthrough]];
	case Token::Type::LBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::RBRACKET: __COUNTER__; [[fallthrough]];
	case Token::Type::COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::DOUBLE_COLON: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMA: __COUNTER__; [[fallthrough]];
	case Token::Type::POINTING: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_START: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_ASM: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_VOID: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNSIGNED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_AS: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_IDENTIFIER_END: __COUNTER__; [[fallthrough]];
	case Token::Type::VARIADIC: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_START: __COUNTER__; [[fallthrough]];
	case Token::Type::MACRO_END: __COUNTER__; [[fallthrough]];
	case Token::Type::COMMENT: __COUNTER__; [[fallthrough]];			// 주석은 파서에서 토큰을 읽으면 안됩니다.
	case Token::Type::COMMENT_BLOCK: __COUNTER__; [[fallthrough]];	// 주석은 파서에서 토큰을 읽으면 안됩니다.
	default:
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. TokenType=%s(%zu) TokenLiteral=`%s`",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());
		_errors.push(ErrorInfo{ ErrorID::NOT_REGISTERED_STATEMENT_TOKEN, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		break;
	}
	}
	constexpr const size_t TOKENTYPE_COUNT = __COUNTER__ - TOKENTYPE_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == TOKENTYPE_COUNT, "parsing statement by token type.");
	return statement;
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseIncludeLibraryStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::MACRO_INCLUDE, u8"이 함수가 호출될때 현재 토큰이 `MACRO_INCLUDE`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	if (ReadNextTokenIf(mcf::Token::Type::LT) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `LT`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	if (ReadNextTokenIf(mcf::Token::Type::KEYWORD_ASM) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `KEYWORD_ASM`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	if (ReadNextTokenIf(mcf::Token::Type::COMMA) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `COMMA`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	if (ReadNextTokenIf(mcf::Token::Type::STRING) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `STRING`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}
	mcf::Token::Data libPath = _currentToken;

	if (ReadNextTokenIf(mcf::Token::Type::GT) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `GT`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}
	return mcf::AST::Statement::IncludeLibrary::Make(libPath);
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseTypedefStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_TYPEDEF, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_TYPEDEF`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	ReadNextToken();
	mcf::AST::Statement::Typedef::SignaturePointer signature = ParseVariableSignatureIntermediate();
	if (signature.get() == nullptr || signature->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Typedef 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	if (ReadNextTokenIf(mcf::Token::Type::SEMICOLON) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `SEMICOLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	return mcf::AST::Statement::Typedef::Make(std::move(signature));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseExternStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_EXTERN, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_EXTERN`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	ReadNextToken();
	mcf::AST::Intermediate::FunctionSignature::Pointer signature = ParseFunctionSignatureIntermediate();
	if (signature.get() == nullptr || signature->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Extern 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	if (ReadNextTokenIf(mcf::Token::Type::SEMICOLON) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `SEMICOLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	return mcf::AST::Statement::Extern::Make(std::move(signature));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseLetStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_LET, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_LET`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	ReadNextToken();
	mcf::AST::Intermediate::VariableSignature::Pointer signature = ParseVariableSignatureIntermediate();
	if (signature.get() == nullptr || signature->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Let 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	mcf::AST::Expression::Pointer expression;
	if (ReadNextTokenIf(mcf::Token::Type::ASSIGN) == true)
	{
		ReadNextToken();
		expression = ParseExpression(Precedence::LOWEST);
		if (expression.get() == nullptr || expression->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"Let 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return mcf::AST::Statement::Invalid::Make();
		}
	}

	if (ReadNextTokenIf(mcf::Token::Type::SEMICOLON) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `SEMICOLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	return mcf::AST::Statement::Let::Make(std::move(signature), std::move(expression));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseBlockStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::LBRACE, u8"이 함수가 호출될때 현재 토큰이 `LBRACE`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	mcf::AST::Statement::PointerVector statements;
	while (ReadNextTokenIf(mcf::Token::Type::RBRACE) == false)
	{
		if (ReadNextTokenIf(mcf::Token::Type::END_OF_FILE) == true)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"Block 명령문 파싱중 파일의 끝에 도달 했습니다. Block 명령문은 반드시 RBRACE('}')로 끝나야 합니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return mcf::AST::Statement::Invalid::Make();
		}

		ReadNextToken();
		statements.emplace_back(ParseStatement());
		if (statements.back().get() == nullptr || statements.back()->GetStatementType() == mcf::AST::Statement::Type::INVALID)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"Block 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return mcf::AST::Statement::Invalid::Make();
		}
	}

	return mcf::AST::Statement::Block::Make(std::move(statements));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseReturnStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_RETURN, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_RETURN`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	ReadNextToken();
	mcf::AST::Expression::Pointer returnValue = ParseExpression(Precedence::LOWEST);
	if (returnValue.get() == nullptr || returnValue->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Return 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	if (ReadNextTokenIf(mcf::Token::Type::SEMICOLON) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `SEMICOLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	return mcf::AST::Statement::Return::Make(std::move(returnValue));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseFuncStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_FUNC, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_FUNC`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	mcf::AST::Intermediate::FunctionSignature::Pointer signature = ParseFunctionSignatureIntermediate();
	if (signature.get() == nullptr || signature->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Func 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	ReadNextToken();
	mcf::AST::Statement::Pointer statementBlock = ParseBlockStatement();
	if (statementBlock.get() == nullptr || statementBlock->GetStatementType() != mcf::AST::Statement::Type::BLOCK)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Func 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}
	return mcf::AST::Statement::Func::Make(std::move(signature), std::move(mcf::AST::Statement::Block::Pointer(static_cast<mcf::AST::Statement::Block*>(statementBlock.release()))));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseMainStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_MAIN, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_MAIN`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	ReadNextToken();
	mcf::AST::Intermediate::FunctionParams::Pointer params = ParseFunctionParamsIntermediate();
	if (params.get() == nullptr || params->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Main 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::POINTING) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `POINTING`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::KEYWORD_VOID) == true)
	{
		ReadNextToken();
		mcf::AST::Statement::Pointer statementBlock = ParseBlockStatement();
		if (statementBlock.get() == nullptr || statementBlock->GetStatementType() != mcf::AST::Statement::Type::BLOCK)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"Main 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return mcf::AST::Statement::Invalid::Make();
		}

		return mcf::AST::Statement::Main::Make(std::move(params), nullptr, std::move(mcf::AST::Statement::Block::Pointer(static_cast<mcf::AST::Statement::Block*>(statementBlock.release()))));
	}

	ReadNextToken();
	mcf::AST::Intermediate::TypeSignature::Pointer returnType = ParseTypeSignatureIntermediate();
	if (returnType.get() == nullptr || returnType->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Main 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	ReadNextToken();
	mcf::AST::Statement::Pointer statementBlock = ParseBlockStatement();
	if (statementBlock.get() == nullptr || statementBlock->GetStatementType() != mcf::AST::Statement::Type::BLOCK)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Main 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}

	return mcf::AST::Statement::Main::Make(std::move(params), std::move(returnType), std::move(mcf::AST::Statement::Block::Pointer(static_cast<mcf::AST::Statement::Block*>(statementBlock.release()))));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseExpressionStatement(void) noexcept
{
	mcf::AST::Expression::Pointer expression = ParseExpression(Precedence::LOWEST);
	if (expression.get() == nullptr || expression->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Expression 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::ASSIGN) == true)
	{
		constexpr const size_t EXPRESSION_COUNT_BEGIN = __COUNTER__;
		switch (expression->GetExpressionType())
		{
		case AST::Expression::Type::IDENTIFIER: __COUNTER__;
			break;

		case AST::Expression::Type::INTEGER: __COUNTER__; [[fallthrough]];
		case AST::Expression::Type::STRING: __COUNTER__; [[fallthrough]];
		case AST::Expression::Type::PREFIX: __COUNTER__; [[fallthrough]];
		case AST::Expression::Type::GROUP: __COUNTER__; [[fallthrough]];	
		case AST::Expression::Type::INFIX: __COUNTER__; [[fallthrough]];	
		case AST::Expression::Type::CALL: __COUNTER__; [[fallthrough]];	
		case AST::Expression::Type::AS: __COUNTER__; [[fallthrough]];
		case AST::Expression::Type::INDEX: __COUNTER__; [[fallthrough]];
		case AST::Expression::Type::INITIALIZER: __COUNTER__; [[fallthrough]];	
		case AST::Expression::Type::MAP_INITIALIZER: __COUNTER__; [[fallthrough]];
		default:
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"AssignExpression 명령문 파싱중 예상치 못한 left expression 타입이 들어왔습니다. LeftExpressionType=%s(%zu)",
				mcf::AST::Expression::CONVERT_TYPE_TO_STRING(expression->GetExpressionType()), mcf::ENUM_INDEX(expression->GetExpressionType()));
			_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}
		}
		constexpr const size_t EXPRESSION_COUNT = __COUNTER__ - EXPRESSION_COUNT_BEGIN;
		static_assert(static_cast<size_t>(mcf::AST::Expression::Type::COUNT) == EXPRESSION_COUNT, "Expression type count is changed. this SWITCH need to be changed as well.");

		ReadNextToken();
		mcf::AST::Expression::Pointer rightExpression = ParseExpression(Precedence::LOWEST);
		if (rightExpression.get() == nullptr || rightExpression->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"AssignExpression 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}

		if (ReadNextTokenIf(mcf::Token::Type::SEMICOLON) == false)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `SEMICOLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		return mcf::AST::Statement::AssignExpression::Make(std::move(expression), std::move(rightExpression));
	}

	if (ReadNextTokenIf(mcf::Token::Type::SEMICOLON) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `SEMICOLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Statement::Expression::Make(std::move(expression));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseUnusedStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_UNUSED, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_UNUSED`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	if (ReadNextTokenIf(mcf::Token::Type::LPAREN) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `LPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	mcf::AST::Expression::Identifier::PointerVector identifiers;
	if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == true)
	{
		return mcf::AST::Statement::Unused::Make(std::move(identifiers));
	}

	if (ReadNextTokenIf(mcf::Token::Type::IDENTIFIER) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `IDENTIFIER`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}
	identifiers.emplace_back(mcf::AST::Expression::Identifier::Make(_currentToken));

	while (ReadNextTokenIf(mcf::Token::Type::COMMA) == true)
	{
		if (ReadNextTokenIf(mcf::Token::Type::END_OF_FILE) == true)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"Unused 명령문 파싱중 파일의 끝에 도달 했습니다. Unused 명령문 반드시 RPAREN(')')로 끝나야 합니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == true)
		{
			return mcf::AST::Statement::Unused::Make(std::move(identifiers));
		}

		if (ReadNextTokenIf(mcf::Token::Type::IDENTIFIER) == false)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `IDENTIFIER`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}
		identifiers.emplace_back(mcf::AST::Expression::Identifier::Make(_currentToken));
	}

	if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::SEMICOLON) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `SEMICOLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Statement::Unused::Make(std::move(identifiers));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseWhileStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_WHILE, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_WHILE`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	if (ReadNextTokenIf(mcf::Token::Type::LPAREN) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `LPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	ReadNextToken();
	mcf::AST::Expression::Pointer conditionExpression = ParseExpression(mcf::Parser::Precedence::LOWEST);
	if (conditionExpression.get() == nullptr || conditionExpression->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"While 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::LBRACE) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `LBRACE`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	mcf::AST::Statement::Pointer statementBlock = ParseBlockStatement();
	if (statementBlock.get() == nullptr || statementBlock->GetStatementType() != mcf::AST::Statement::Type::BLOCK)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Func 명령문 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_STATEMENT_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return mcf::AST::Statement::Invalid::Make();
	}
	return mcf::AST::Statement::While::Make(std::move(conditionExpression), std::move(mcf::AST::Statement::Block::Pointer(static_cast<mcf::AST::Statement::Block*>(statementBlock.release()))));
}

mcf::AST::Statement::Pointer mcf::Parser::Object::ParseBreakStatement(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_BREAK, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_BREAK`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	const mcf::Token::Data token = _currentToken;

	if (ReadNextTokenIf(mcf::Token::Type::SEMICOLON) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `SEMICOLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}
	return mcf::AST::Statement::Break::Make(_currentToken);
}

mcf::AST::Intermediate::Variadic::Pointer mcf::Parser::Object::ParseVariadicIntermediate(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::VARIADIC, u8"이 함수가 호출될때 현재 토큰이 `VARIADIC`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	if (ReadNextTokenIf(mcf::Token::Type::IDENTIFIER) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `IDENTIFIER`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Intermediate::Variadic::Make(mcf::AST::Expression::Identifier::Make(_currentToken));
}

mcf::AST::Intermediate::TypeSignature::Pointer mcf::Parser::Object::ParseTypeSignatureIntermediate(void) noexcept
{
	MCF_DEBUG_ASSERT(IsCurrentTokenAny({ mcf::Token::Type::IDENTIFIER, mcf::Token::Type::KEYWORD_UNSIGNED }), u8"이 함수가 호출될때 현재 토큰이 `IDENTIFIER` 또는 `KEYWORD_UNSIGNED`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	bool isUnsigned = false;
	if (_currentToken.Type == mcf::Token::Type::KEYWORD_UNSIGNED)
	{
		isUnsigned = true;
		if (ReadNextTokenIf( mcf::Token::Type::IDENTIFIER ) == false)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `IDENTIFIER`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}
	}
	mcf::AST::Expression::Pointer signature = mcf::AST::Expression::Identifier::Make(_currentToken);

	while (ReadNextTokenIf(mcf::Token::Type::LBRACKET) == true)
	{
		if (ReadNextTokenIf(mcf::Token::Type::END_OF_FILE) == true)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"TypeSignature 표현식 파싱중 파일의 끝에 도달 했습니다. Index 표현식은 반드시 RBRACKET(']')로 끝나야 합니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		signature = ParseIndexExpression(std::move(signature));
		if (signature.get() == nullptr || signature->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"TypeSignature 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}
	}
	return mcf::AST::Intermediate::TypeSignature::Make(isUnsigned, std::move(signature));
}

mcf::AST::Intermediate::VariableSignature::Pointer mcf::Parser::Object::ParseVariableSignatureIntermediate(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::IDENTIFIER, u8"이 함수가 호출될때 현재 토큰이 `IDENTIFIER`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	mcf::AST::Expression::Identifier::Pointer name = mcf::AST::Expression::Identifier::Make(_currentToken);
	if (ReadNextTokenIf(mcf::Token::Type::COLON) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `COLON`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	ReadNextToken();
	mcf::AST::Intermediate::TypeSignature::Pointer typeSignature = ParseTypeSignatureIntermediate();
	if (typeSignature.get() == nullptr || typeSignature->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"VariableSignature 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	return mcf::AST::Intermediate::VariableSignature::Make(std::move(name), std::move(typeSignature));
}

mcf::AST::Intermediate::FunctionParams::Pointer mcf::Parser::Object::ParseFunctionParamsIntermediate(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::LPAREN, u8"이 함수가 호출될때 현재 토큰이 `LPAREN`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	if (ReadNextTokenIf(mcf::Token::Type::KEYWORD_VOID) == true)
	{
		if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == false)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		return mcf::AST::Intermediate::FunctionParams::Make();
	}

	if (ReadNextTokenIf(mcf::Token::Type::VARIADIC) == true)
	{
		mcf::AST::Intermediate::Variadic::Pointer variadic = ParseVariadicIntermediate();

		if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == false)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		return mcf::AST::Intermediate::FunctionParams::Make(std::vector<mcf::AST::Intermediate::VariableSignature::Pointer>(), std::move(variadic));
	}

	ReadNextToken();
	mcf::AST::Intermediate::VariableSignature::PointerVector params;
	mcf::AST::Intermediate::VariableSignature::Pointer firstParam = ParseVariableSignatureIntermediate();
	if (firstParam.get() == nullptr || firstParam->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"FunctionParams value 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}
	params.emplace_back(std::move(firstParam));

	if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == true)
	{
		return mcf::AST::Intermediate::FunctionParams::Make(std::move(params), nullptr);
	}

	while (ReadNextTokenIf(mcf::Token::Type::COMMA) == true)
	{
		if (ReadNextTokenIf(mcf::Token::Type::END_OF_FILE) == true)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"FunctionParams 중간 표현식 파싱중 파일의 끝에 도달 했습니다. FunctionParams 중간 표현식은 반드시 RPAREN(')')로 끝나야 합니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		// if next token is the start token of VariableSignature
		if (ReadNextTokenIf(mcf::Token::Type::IDENTIFIER) == true)
		{
			mcf::AST::Intermediate::VariableSignature::Pointer nextParam = ParseVariableSignatureIntermediate();
			if (nextParam.get() == nullptr || nextParam->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
			{
				const std::string message = mcf::Internal::ErrorMessage(u8"FunctionParams value 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
					mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
				_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
				return nullptr;
			}
			params.emplace_back(std::move(nextParam));
		}
		else if (ReadNextTokenIf(mcf::Token::Type::VARIADIC) == true)
		{
			mcf::AST::Intermediate::Variadic::Pointer variadic = ParseVariadicIntermediate();

			if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == false)
			{
				const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
					mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
				_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
				return nullptr;
			}

			return mcf::AST::Intermediate::FunctionParams::Make(std::move(params), std::move(variadic));
		}
		else
		{
			break;
		}
	}

	if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Intermediate::FunctionParams::Make(std::move(params), nullptr);
}

mcf::AST::Intermediate::FunctionSignature::Pointer mcf::Parser::Object::ParseFunctionSignatureIntermediate(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_FUNC, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_FUNC`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	if (ReadNextTokenIf(mcf::Token::Type::IDENTIFIER) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `IDENTIFIER`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}
	mcf::AST::Expression::Identifier::Pointer name = mcf::AST::Expression::Identifier::Make(_currentToken);

	ReadNextToken();
	mcf::AST::Intermediate::FunctionParams::Pointer params = ParseFunctionParamsIntermediate();
	if (params.get() == nullptr || params->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"FunctionSignature value 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::POINTING) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `POINTING`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::KEYWORD_VOID) == true)
	{
		return mcf::AST::Intermediate::FunctionSignature::Make(std::move(name), std::move(params), nullptr);
	}

	ReadNextToken();
	mcf::AST::Intermediate::TypeSignature::Pointer returnType = ParseTypeSignatureIntermediate();
	if (returnType.get() == nullptr || returnType->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"FunctionSignature value 중간 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_INTERMEDIATE_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	return mcf::AST::Intermediate::FunctionSignature::Make(std::move(name), std::move(params), std::move(returnType));
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

	case Token::Type::MINUS: __COUNTER__; [[fallthrough]];
	case Token::Type::BANG: __COUNTER__; [[fallthrough]];
	case Token::Type::AMPERSAND: __COUNTER__;
		expression = ParsePrefixExpression();
		break;

	case Token::Type::LPAREN: __COUNTER__;
		expression = ParseGroupExpression();
		break;

	case Token::Type::LBRACE: __COUNTER__;
		expression = ParseInitializerExpression();
		break;

	case Token::Type::KEYWORD_UNSIGNED: __COUNTER__;
		MCF_DEBUG_BREAK(u8"구현 필요");
		break;

	case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
	case Token::Type::ASSIGN: __COUNTER__; [[fallthrough]];
	case Token::Type::PLUS: __COUNTER__; [[fallthrough]];
	case Token::Type::ASTERISK: __COUNTER__; [[fallthrough]];
	case Token::Type::SLASH: __COUNTER__; [[fallthrough]];
	case Token::Type::EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::NOT_EQUAL: __COUNTER__; [[fallthrough]];
	case Token::Type::LT: __COUNTER__; [[fallthrough]];
	case Token::Type::GT: __COUNTER__; [[fallthrough]];
	case Token::Type::RPAREN: __COUNTER__; [[fallthrough]];
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
	case Token::Type::KEYWORD_LET: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_FUNC: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_MAIN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_VOID: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_RETURN: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_UNUSED: __COUNTER__; [[fallthrough]];
	case Token::Type::KEYWORD_AS: __COUNTER__; [[fallthrough]];
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
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. TokenType=%s(%zu) TokenLiteral=`%s`",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());
		_errors.push(ErrorInfo{ ErrorID::NOT_REGISTERED_EXPRESSION_TOKEN, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		break;
	}
	}
	constexpr const size_t EXPRESSION_TOKEN_COUNT = __COUNTER__ - EXPRESSION_TOKEN_COUNT_BEGIN;
	static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == EXPRESSION_TOKEN_COUNT, "parsing expression by token type.");

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
		case Token::Type::NOT_EQUAL: __COUNTER__; [[fallthrough]];
		case Token::Type::LT: __COUNTER__; [[fallthrough]];
		case Token::Type::GT: __COUNTER__; 
			ReadNextToken();
			expression = ParseInfixExpression(std::move(expression));
			break;

		case Token::Type::LPAREN: __COUNTER__;
			ReadNextToken();
			expression = ParseCallExpression(std::move(expression));
			break;

		case Token::Type::LBRACKET: __COUNTER__;
			ReadNextToken();
			expression = ParseIndexExpression(std::move(expression));
			break;
		
		case Token::Type::KEYWORD_AS: __COUNTER__;
			ReadNextToken();
			expression = ParseAsExpression(std::move(expression));
			break;

		case Token::Type::END_OF_FILE: __COUNTER__; [[fallthrough]];
		case Token::Type::IDENTIFIER: __COUNTER__; [[fallthrough]];
		case Token::Type::INTEGER: __COUNTER__; [[fallthrough]];
		case Token::Type::STRING: __COUNTER__; [[fallthrough]];
		case Token::Type::ASSIGN: __COUNTER__; [[fallthrough]];
		case Token::Type::BANG: __COUNTER__; [[fallthrough]];
		case Token::Type::AMPERSAND: __COUNTER__; [[fallthrough]];
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
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"예상치 못한 값이 들어왔습니다. 확인 해 주세요. TokenType=%s(%zu) TokenLiteral=`%s`",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());
			_errors.push(ErrorInfo{ ErrorID::NOT_REGISTERED_EXPRESSION_TOKEN, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			break;
		}
		}
		constexpr const size_t INFIX_COUNT = __COUNTER__ - INFIX_COUNT_BEGIN;
		static_assert(static_cast<size_t>(mcf::Token::Type::COUNT) == INFIX_COUNT, "parsing infix expression by token type.");
	}
	return expression.get() == nullptr ? mcf::AST::Expression::Invalid::Make() : std::move(expression);
}

mcf::AST::Expression::Prefix::Pointer mcf::Parser::Object::ParsePrefixExpression(void) noexcept
{
	const mcf::Token::Data prefixOperator = _currentToken;
	ReadNextToken();
	mcf::AST::Expression::Pointer right = ParseExpression(Precedence::PREFIX);
	if (right.get() == nullptr || right->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Prefix 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}
	return mcf::AST::Expression::Prefix::Make(prefixOperator, std::move(right));
}

mcf::AST::Expression::Group::Pointer mcf::Parser::Object::ParseGroupExpression(void) noexcept
{
	MCF_DEBUG_ASSERT( _currentToken.Type == mcf::Token::Type::LPAREN, u8"이 함수가 호출될때 현재 토큰이 `LPAREN`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING( _currentToken.Type ), mcf::ENUM_INDEX( _currentToken.Type ), _currentToken.Literal.c_str() );

	ReadNextToken();
	mcf::AST::Expression::Pointer expreesion = ParseExpression(Precedence::LOWEST);
	if ( expreesion.get() == nullptr || expreesion->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Group 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Expression::Group::Make(std::move(expreesion));
}

mcf::AST::Expression::Infix::Pointer mcf::Parser::Object::ParseInfixExpression(mcf::AST::Expression::Pointer&& left) noexcept
{
	const mcf::Token::Data infixOperator = _currentToken;
	const Precedence precedence = GetCurrentTokenPrecedence();
	ReadNextToken();
	mcf::AST::Expression::Pointer right = ParseExpression(precedence);
	if (right.get() == nullptr || right->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Index 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}
	return mcf::AST::Expression::Infix::Make(std::move(left), infixOperator, std::move(right));
}

mcf::AST::Expression::Call::Pointer mcf::Parser::Object::ParseCallExpression(mcf::AST::Expression::Pointer&& left) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::LPAREN, u8"이 함수가 호출될때 현재 토큰이 `LPAREN`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	mcf::AST::Expression::PointerVector params;
	if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == true)
	{
		return mcf::AST::Expression::Call::Make(std::move(left), std::move(params));
	}

	ReadNextToken();
	params.emplace_back(ParseExpression(Precedence::LOWEST));
	if (params.back().get() == nullptr || params.back()->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Call 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	while (ReadNextTokenIf(mcf::Token::Type::COMMA) == true)
	{
		if (ReadNextTokenIf(mcf::Token::Type::END_OF_FILE) == true)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"Call 표현식 파싱중 파일의 끝에 도달 했습니다. Call 표현식은 반드시 RPAREN(')')로 끝나야 합니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == true)
		{
			return mcf::AST::Expression::Call::Make(std::move(left), std::move(params));
		}

		ReadNextToken();
		params.emplace_back(ParseExpression(Precedence::LOWEST));
		if (params.back().get() == nullptr || params.back()->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"Call 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}
	}

	if (ReadNextTokenIf(mcf::Token::Type::RPAREN) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RPAREN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Expression::Call::Make(std::move(left), std::move(params));
}

mcf::AST::Expression::Index::Pointer mcf::Parser::Object::ParseIndexExpression(mcf::AST::Expression::Pointer&& left) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::LBRACKET, u8"이 함수가 호출될때 현재 토큰이 `LBRACKET`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	if (ReadNextTokenIf(mcf::Token::Type::RBRACKET) == true)
	{
		return mcf::AST::Expression::Index::Make(std::move(left), nullptr);
	}

	ReadNextToken();
	mcf::AST::Expression::Pointer index = ParseExpression(Precedence::LOWEST);
	if (index.get() == nullptr || index->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Index 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::RBRACKET) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RBRACE`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Expression::Index::Make(std::move(left), std::move(index));
}

mcf::AST::Expression::As::Pointer mcf::Parser::Object::ParseAsExpression(mcf::AST::Expression::Pointer&& left) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::KEYWORD_AS, u8"이 함수가 호출될때 현재 토큰이 `KEYWORD_AS`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	ReadNextToken();
	mcf::AST::Intermediate::TypeSignature::Pointer typeCastedAs = ParseTypeSignatureIntermediate();
	if (typeCastedAs.get() == nullptr || typeCastedAs->GetIntermediateType() == mcf::AST::Intermediate::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"As 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	return mcf::AST::Expression::As::Make(std::move(left), std::move(typeCastedAs));
}

mcf::AST::Expression::Initializer::Pointer mcf::Parser::Object::ParseInitializerExpression(void) noexcept
{
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::LBRACE, u8"이 함수가 호출될때 현재 토큰이 `LBRACE`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	ReadNextToken();
	mcf::AST::Expression::PointerVector keyList;
	keyList.emplace_back(ParseExpression(Precedence::LOWEST));
	if (keyList.back().get() == nullptr || keyList.back()->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"Initializer key 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::RBRACE) == true)
	{
		return mcf::AST::Expression::Initializer::Make(std::move(keyList));
	}

	if (ReadNextTokenIf(mcf::Token::Type::ASSIGN) == true)
	{
		return ParseMapInitializerExpression(std::move(keyList));
	}

	while (ReadNextTokenIf(mcf::Token::Type::COMMA) == true)
	{
		if (ReadNextTokenIf(mcf::Token::Type::END_OF_FILE) == true)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"Initializer 표현식 파싱중 파일의 끝에 도달 했습니다. Initializer 표현식은 반드시 RBRACE('}')로 끝나야 합니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		if (ReadNextTokenIf(mcf::Token::Type::RBRACE) == true)
		{
			return mcf::AST::Expression::Initializer::Make(std::move(keyList));
		}

		ReadNextToken();
		keyList.emplace_back(ParseExpression(Precedence::LOWEST));
		if (keyList.back().get() == nullptr || keyList.back()->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"MapInitializer key 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}
	}

	if (ReadNextTokenIf(mcf::Token::Type::RBRACE) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RBRACE`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Expression::Initializer::Make(std::move(keyList));
}

// 이 함수를 호출 하려면 첫번째 키까지 사전에 파싱을 하여야 합니다 (ParseInitializerExpression 참고)
mcf::AST::Expression::MapInitializer::Pointer mcf::Parser::Object::ParseMapInitializerExpression(mcf::AST::Expression::PointerVector&& keyList) noexcept
{
	MCF_DEBUG_ASSERT(keyList.size() == 1, u8"인자로 받은 keyList는 한개의 키를 들고 있어야 합니다.");
	MCF_DEBUG_ASSERT(keyList[0].get() != nullptr, u8"keyList[0]는 nullptr 여선 안됩니다.");
	MCF_DEBUG_ASSERT(_currentToken.Type == mcf::Token::Type::ASSIGN, u8"이 함수가 호출될때 현재 토큰이 `ASSIGN`여야만 합니다! 현재 TokenType=%s(%zu) TokenLiteral=`%s`",
		mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type), mcf::ENUM_INDEX(_currentToken.Type), _currentToken.Literal.c_str());

	ReadNextToken();
	mcf::AST::Expression::PointerVector valueList;
	valueList.emplace_back(ParseExpression(Precedence::LOWEST));
	if (valueList.back().get() == nullptr || valueList.back()->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"MapInitializer value 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
		_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
		return nullptr;
	}

	if (ReadNextTokenIf(mcf::Token::Type::RBRACE) == true)
	{
		return mcf::AST::Expression::MapInitializer::Make(std::move(keyList), std::move(valueList));
	}

	while (ReadNextTokenIf(mcf::Token::Type::COMMA) == true)
	{
		if (ReadNextTokenIf(mcf::Token::Type::END_OF_FILE) == true)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"MapInitializer 표현식 파싱중 파일의 끝에 도달 했습니다. MapInitializer 표현식은 반드시 RBRACE('}')로 끝나야 합니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		if (ReadNextTokenIf(mcf::Token::Type::RBRACE) == true)
		{
			return mcf::AST::Expression::MapInitializer::Make(std::move(keyList), std::move(valueList));
		}

		ReadNextToken();
		keyList.emplace_back(ParseExpression(Precedence::LOWEST));
		if (keyList.back().get() == nullptr || keyList.back()->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"MapInitializer key 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}

		if (ReadNextTokenIf(mcf::Token::Type::ASSIGN) == false)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `ASSIGN`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
			_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
			return nullptr;
		}

		ReadNextToken();
		valueList.emplace_back(ParseExpression(Precedence::LOWEST));
		if (valueList.back().get() == nullptr || valueList.back()->GetExpressionType() == mcf::AST::Expression::Type::INVALID)
		{
			const std::string message = mcf::Internal::ErrorMessage(u8"MapInitializer value 표현식 파싱에 실패하였습니다. 파싱 실패 값으로 %s를 받았습니다.",
				mcf::Token::CONVERT_TYPE_TO_STRING(_currentToken.Type));
			_errors.push(ErrorInfo{ ErrorID::FAIL_EXPRESSION_PARSING, _lexer.GetName(), message, _currentToken.Line, _currentToken.Index });
			return nullptr;
		}
	}

	if (ReadNextTokenIf(mcf::Token::Type::RBRACE) == false)
	{
		const std::string message = mcf::Internal::ErrorMessage(u8"다음 토큰은 `RBRACE`타입여야만 합니다. 실제 값으로 %s를 받았습니다.",
			mcf::Token::CONVERT_TYPE_TO_STRING(_nextToken.Type));
		_errors.push(ErrorInfo{ ErrorID::UNEXPECTED_NEXT_TOKEN, _lexer.GetName(), message, _nextToken.Line, _nextToken.Index });
		return nullptr;
	}

	return mcf::AST::Expression::MapInitializer::Make(std::move(keyList), std::move(valueList));
}