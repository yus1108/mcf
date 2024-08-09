# mcf
This is a toy compiler project named as mcf (making compiler is fun) aiming to make a compiler specialized for game development

## `Terms` :

`...` : this can be replaced in practice as a series of any symbols (tokens|expressions|statements) with at least one symbol.

`TOKEN` : the highlighted and capitalized identifier represents a token

`<Expression>` : the value between braces represents a expression

`[Statement]` : the value between brackets represents a statement

`{ ... }` : this represents that `...` is optional when parsing.

`{ ... }+` : this represents that `...` must be there at least ONE or MORE.

`{ ... }*` : this represents that `...` can be there NONE or MORE.

`[prerequisite: TOKEN]` : this represents that `TOKEN` must be required before parsing next symbol.

`[expectnext: TOKEN]` : this represents that `TOKEN` must be required for next symbol.

---

## `<Expression>` :

### `<Identifier>`

#### PARSER :
`IDENTIFIER`

---

### `<Literal>`

#### PARSER :
`LITERAL`

---

### `<String>`

#### PARSER :
`STRING`

---

### `<Index>`

#### PARSER :
`<Expression>` {`LBRACKET` {`<Expression>`} `RBRACKET`}+

---

## `<INTERMEDIATE>` :

### `<MapInitializer>`

#### PARSER :
`LBRACE` `<Expression>` `ASSIGN` `<Expression>` {`COMMA` {`<Expression>` `ASSIGN` `<Expression>`}}* `RBRACE`

---

### `<TypeSignature>`

#### PARSER :
1. `<Identifier>`
2. `<Index>`

#### EVALUATOR :

`<Identifier>`: identifier must be registered as type unless used for `[Typedef]`

`<Index>`: for this index expression, the leftmost expression must be TYPE identifier, and the index value must be either the integer or opted out.

---

### `<VariableSignature>`

#### PARSER :
`<Identifier>` `COLON` `<TypeSignature>`

---

### `<FunctionSignature>`

#### PARSER :
[prerequisite: `KEYWORD_FUNC`] `<Identifier>` `<FunctionParams>` `POINTING` `KEYWORD_VOID`

---

### `<FunctionParams>`

#### PARSER :
1. `LPAREN` `KEYWORD_VOID` `RPAREN`
2. `LPAREN` `VARIADIC` `RPAREN`
3. `LPAREN` {`<VariableSignature>` {`COMMA`}}+ {[prerequisite: `COMMA`] `VARIADIC`} `RPAREN`

---

### `<Statements>`

#### PARSER :
[prerequisite: `LBRACE`] {`[Statement]`}* [expectnext: `RBRACE`]

---

## `[Statement]` : 

### `[IncludeLibrary]`

#### PARSER :
`MACRO_INCLUDE` `LT` `KEYWORD_ASM` `COMMA` `STRING` `GT`

#### EVALUATOR:
* `KEYWORD_LIB` : this represent that this will include a library file without any header.
* `STRING` : this represent the path for the binary library file.

---

### `[Typedef]`

#### PARSER :
`KEYWORD_TYPEDEF` `<VariableSignature>` {`POINTING` `KEYWORD_BIND` `<MapInitializer>`} `SEMICOLON`

#### EVALUATOR:
* `<VariableSignature>` : represent a new type equivalent to source type with the size specified if any and/or the values specified if any.
* {`POINTING` `KEYWORD_BIND` `<MapInitializer>`} : represent values specified if any. If not, it will inherit values from the source type.
* `<MapInitializer>` : for this expression, a key for each item must be identifier.

---

### `[Extern]`

#### PARSER :
`KEYWORD_EXTERN` `KEYWORD_ASM` `KEYWORD_FUNC` `<FunctionSignature>` `SEMICOLON`

#### EVALUATOR:
* `KEYWORD_ASM`: when calling this function, it will pass calling without function description.
* `<FunctionSignature>` : the function signature will be written as extern in the taget assembly.

---

### `[Let]`

#### PARSER :
`KEYWORD_LET` `<VariableSignature>` {`ASSIGN` `<Expression>`} `SEMICOLON`

#### EVALUATOR:
{`ASSIGN` `<Expression>`} : if this phrase exist, the variable signature will be stored as data with initialized value; otherwise, it will be stored as bss.

---

### `[Func]`

#### PARSER :
`KEYWORD_FUNC` `<FunctionSignature>` `LBRACE` `<Statements>` `RBRACE`

#### EVALUATOR:
* `<FunctionSignature>`: the signature is stored to code section.
* `<Statements>`: `[Return]` is required at the end of statements if the signature has return type, and both type must be matched.

---

### `[Return]`

#### PARSER :
`KEYWORD_RETURN` `<Expression>` `SEMICOLON`

#### EVALUATOR:
`<Expression>`: this expression must have value