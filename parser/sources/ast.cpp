#include "pch.h"
#include "ast.h"

const size_t mcf::ast::program::get_statement_size( void ) const noexcept
{
	return _statements.size();
}

mcf::pointer<mcf::ast::statement> mcf::ast::program::get_statement_at( const size_t index )
{
	return mcf::pointer<mcf::ast::statement>(_statements[index].get());
}

const mcf::pointer<mcf::ast::statement> mcf::ast::program::get_statement_at( const size_t index ) const
{
	return mcf::pointer<mcf::ast::statement>(_statements[index].get());
}