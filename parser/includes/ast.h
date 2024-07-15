#pragma once
#include <vector>
#include <memory>
#include "lexer.h"

namespace mcf
{
	namespace ast
	{
		enum class node_type : unsigned char
		{
			invalid = 0,

			expression,
			statement,

			// 이 밑으로는 수정하면 안됩니다.
			count,
		};

		class node
		{
		public:
			virtual ~node(void) noexcept {}
			virtual const mcf::ast::node_type	get_node_type(void) const noexcept = 0;
			virtual const std::string			convert_to_string(void) const noexcept = 0;
		};

		enum class statement_type : unsigned char
		{
			invalid = 0,

			variable_declaration,	// <data_type> identifier [optional: assign <expression>] semicolon
			variable_assignment,	// identifier assign <expression> semicolon

			// 이 밑으로는 수정하면 안됩니다.
			count,
		};

		class statement : public node
		{
		public: 
					virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept = 0;
			inline	virtual const mcf::ast::node_type		get_node_type(void) const noexcept override final { return mcf::ast::node_type::statement; }
		};

		enum class expression_type : unsigned char
		{
			invalid = 0,

			literal,	// integer_32bit
			identifier, // identifier
			data_type,	// keyword_int32

			// plus|minus <literal>
			// plus|minus <identifier>
			prefix,
			infix,	// <expression> plus|minus|asterisk|slash <expression>

			// 이 밑으로는 수정하면 안됩니다.
			count,
		};

		class expression : public node
		{
		public: 
					virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept = 0;
			inline	virtual const mcf::ast::node_type		get_node_type(void) const noexcept override final { return mcf::ast::node_type::expression; }
		};

		class program final
		{
		public:
			explicit program(void) noexcept = default;
			explicit program(std::vector<const mcf::ast::statement*> statements) noexcept;

			inline	const size_t				get_statement_count(void) const noexcept { return _count; }
					const mcf::ast::statement*	get_statement_at(const size_t index) const noexcept;

			const std::string convert_to_string(const bool isAddNewLineForEachStatement = true) const noexcept;

		private:
			using unique_statement = std::unique_ptr <const mcf::ast::statement>;
			using statement_array = std::unique_ptr<unique_statement[]>;

			statement_array	_statements;
			size_t			_count = 0;
		};

		class literal_expession final : public expression
		{
		public: 
			explicit literal_expession(void) noexcept = default;
			explicit literal_expession(const mcf::token& token) noexcept : _token(token) {}

			inline const mcf::token& get_token(void) const noexcept { return _token; }

			inline virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::literal; }
			inline virtual const std::string				convert_to_string(void) const noexcept override final { return _token.Literal; }

		private:
			const mcf::token	_token = { token_type::invalid, std::string() }; // { token_type::integer_32bit, literal }
		};

		class identifier_expression final : public expression
		{
		public:
			explicit identifier_expression(void) noexcept = default;
			explicit identifier_expression(const mcf::token& token) noexcept : _token(token) {}

			inline const mcf::token&  get_token( void ) const noexcept { return _token; }

			inline	virtual const mcf::ast::expression_type get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::identifier; }
			inline virtual const std::string				convert_to_string(void) const noexcept override final { return _token.Literal; }

		private:
			const mcf::token _token = { token_type::invalid, std::string() }; // { token_type::identifier, identifier }
		};

		class data_type_expression final : public expression
		{
		public:
			explicit data_type_expression(void) noexcept = default;
			explicit data_type_expression(const mcf::token& token) noexcept : _token(token) {}

			inline const mcf::token_type get_type(void) const noexcept { return _token.Type; }

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::data_type; }
			inline	virtual const std::string				convert_to_string(void) const noexcept override final { return _token.Literal; }

		private:
			const mcf::token _token = { token_type::invalid, std::string() }; // { token_type::keyword, "int32" }
		};

		class prefix_expression final : public expression
		{
		public:
			explicit prefix_expression(void) noexcept = default;
			explicit prefix_expression(const mcf::token& prefix, const mcf::ast::expression* targetExpression) noexcept 
				: _prefixOperator(prefix), _targetExpression(targetExpression) {}

			inline const mcf::token&			get_prefix_operator_token(void) const noexcept { return _prefixOperator; }
			inline const mcf::ast::expression*	get_target_expression(void) const noexcept { return _targetExpression.get(); }

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::prefix; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			using expression = std::unique_ptr<const mcf::ast::expression>;

			const mcf::token _prefixOperator = { token_type::invalid, std::string() };	// { prefix_operator, literal }
			const expression _targetExpression;									// <expression>
		};

		class infix_expression final : public expression
		{
		public:
			explicit infix_expression(void) noexcept = default;
			explicit infix_expression(const mcf::ast::expression* left, const mcf::token& infix, const mcf::ast::expression* right) noexcept
				: _left(left), _infixOperator(infix), _right(right) {}

			inline const mcf::token&			get_infix_operator_token(void) const noexcept { return _infixOperator; }
			inline const mcf::ast::expression*	get_left_expression(void) const noexcept { return _left.get(); }
			inline const mcf::ast::expression*	get_right_expression(void) const noexcept { return _right.get(); }

			inline	virtual const mcf::ast::expression_type	get_expression_type(void) const noexcept override final { return mcf::ast::expression_type::infix; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			using expression = std::unique_ptr<const mcf::ast::expression>;

			const mcf::token _infixOperator = { token_type::invalid, std::string() };	// { infix_operator, literal }
			const expression _left;														// <expression>
			const expression _right;													// <expression>
		};

		class variable_declaration_statement final : public statement
		{
		public:
			explicit variable_declaration_statement(void) noexcept = default;
			explicit variable_declaration_statement(const mcf::ast::data_type_expression& dataType,
													const mcf::ast::identifier_expression& name,
													const mcf::ast::expression* initExpression) noexcept;

			inline const mcf::token_type		get_type(void) const noexcept { return _dataType.get_type(); }
			inline const std::string&			get_name(void) const noexcept { return _name.get_token().Literal; }
			inline const mcf::ast::expression*	get_init_expression(void) const noexcept { return _initExpression.get(); }

			inline	virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept override final { return mcf::ast::statement_type::variable_declaration; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			const mcf::ast::data_type_expression				_dataType;
			const mcf::ast::identifier_expression				_name;
			const std::unique_ptr<const mcf::ast::expression>	_initExpression;
		};

		class variable_assignment_statement final : public statement
		{
		public:
			explicit variable_assignment_statement(void) noexcept = default;
			explicit variable_assignment_statement(const mcf::ast::identifier_expression& name, const mcf::ast::expression* assignExpression) noexcept;

			inline const std::string&			get_name(void) const noexcept { return _name.get_token().Literal; }
			inline const mcf::ast::expression*	get_assign_expression(void) const noexcept { return _assignExpression.get(); }

			inline	virtual const mcf::ast::statement_type	get_statement_type(void) const noexcept override final { return mcf::ast::statement_type::variable_assignment; }
					virtual const std::string				convert_to_string(void) const noexcept override final;

		private:
			const mcf::ast::identifier_expression				_name;
			const std::unique_ptr<const mcf::ast::expression>	_assignExpression;
		};
	}
}
