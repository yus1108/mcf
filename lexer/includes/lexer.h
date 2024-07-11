#pragma once

namespace mcf
{
	enum class token_type : unsigned char
	{
		invalid = 0,
		eof,

		// 식별자 + 리터럴
		identifier,
		integer_32bit,

		// 연산자
		assign,
		plus,
		minus,
		asterisk,
		slash,

		// 구분자
		semicolon,

		// 예약어 구간 시작 (예약어 추가시 예약 구간 종료 바로 위에 추가해주세요)
		keyword_int32,
		// 예약어 구간 종료

		// 이 밑으로는 수정하면 안됩니다.
		count
	};

	struct token final
	{
		mcf::token_type	Type = mcf::token_type::invalid;
		std::string		Literal;
	};

	// 주의: thread-safe 하지 않은 클래스입니다.
	class lexer final {
	public:
		enum class error_token : unsigned char 
		{
			no_error = 0,
			invalid_input_length,
		};

	public:
		explicit lexer(void) noexcept = delete;
		explicit lexer(const std::string& input) noexcept;

		static const mcf::lexer::error_token get_last_error_token(void) noexcept;

		const mcf::token read_next_token(void) noexcept;

	private:
		void				read_next_byte(void) noexcept;
		const std::string	read_keyword_or_identifier(void) noexcept;
		const std::string	read_number(void) noexcept;

	private:
		const std::string	_input				= nullptr;
		size_t				_currentPosition	= 0;
		size_t				_nextPosition		= 0;
		char				_currentByte		= 0;
	};
}