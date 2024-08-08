# mcf
This is a toy compiler project named as mcf (making compiler is fun) aiming to make a compiler specialized for game development

## Terms

`TOKEN`: the value between '`' represents a token
`\<Expression>`: the value between braces represents a expression
`[Statement]`: the value between brackets represents a statement
[optional: ...]: "..." can be token/expression/statement, and this represents that these "..." can be opted out when parsing

## Expressions

### `\<MapInitializer>`

PARSER:
`LBRACE` `\<expression>` `ASSIGN` `\<expression>` `RBRACE`

## Statements

### `[IncludeLibrary]`

PARSER:
`MACRO_INCLUDE` `LT` `KEYWORD_ASM` `COMMA` `STRING` `GT`

EVALUATOR:
`KEYWORD_LIB`: this represent that this will include a library file without any header
`STRING`: this represent the path for the binary library file

### `[Typedef]`

PARSER:
`TYPEDEF` `IDENTIFIER1` `COLON` `IDENTIFIER2` [optional: `LBRACKET` `INTEGER` `RBRACKET`] [optional: `POINTING` `KEYWORD_BIND` `\<MapInitializer>`] `SEMICOLON`

EVALUATOR:
`IDENTIFIER1`: represent a new type equivalent to source type with the size specified if any and/or the values specified if any.
`IDENTIFIER2`: represent a existing source type
[optional: `LBRACKET` `INTEGER` `RBRACKET`]: represent a specified size of source type
[optional: `POINTING` `KEYWORD_BIND` `\<MapInitializer>`]: represent values specified if any. If not, it will inherit values from the source type.
`\<MapInitializer>`: for this expression, a key for each item must be identifier.

### `Extern`