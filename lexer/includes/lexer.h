#pragma once

namespace mcf
{
	enum class token_type : unsigned char
	{
		illegal = 0,
		eof,

		// 식별자 + 리터럴
		identifier,
		integer,

		// 연산자
		assign,
		plus,

		// 구분자
		semicolon,

		// 예약어
		keyword,

		// 이 밑으로는 수정하면 안됩니다.
		count
	};
}