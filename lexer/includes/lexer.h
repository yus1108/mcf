#pragma once

namespace mcf
{
	enum class token_type : unsigned char
	{
		illegal = 0,
		eof,

		// �ĺ��� + ���ͷ�
		identifier,
		integer,

		// ������
		assign,
		plus,

		// ������
		semicolon,

		// �����
		keyword,

		// �� �����δ� �����ϸ� �ȵ˴ϴ�.
		count
	};
}