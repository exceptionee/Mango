grammar Mango;

program : statement* EOF ;

statement
  : modifier=('var' | 'const') variableList ';' #VariableStatement
  | expression ';' #ExpressionStatement
  | ';' #EmptyStatement
  ;

variableList : variable (',' variable)* ;

variable : name=ID (':' type)? ('=' value=expression)? ;

type
  : type '?' #NullableType
  | type '[' ']' #ArrayType
  | left=type '|' right=type #UnionType
  | '(' type ')' #ParenType
  | 'any' #AnyType
  | 'int' #IntType
  | 'float' #FloatType
  | 'string' #StringType
  | 'char' #CharType
  | 'bool' #BoolType
  | 'null' #NullType
  ;

typeList : type (',' type)* ;

expression
  : '(' expr=expression ')' #ParenExpr
  | array=expression '[' index=expression ']' #ArrayAccess
  | literal #LiteralExpr
  | ID #VarExpr
  | expr=expression op=('++' | '--') #PostFixExpr
  | op=('+' | '-' | '++' | '--' | '!') expr=expression #UnaryExpr
  | left=expression op=('*' | '/' | '%') right=expression #SecondaryExpr
  | left=expression op=('+' | '-') right=expression #PrimaryExpr
  | left=expression op=('>' | '<' | '>=' | '<=') right=expression #ComparativeExpr
  | left=expression op=('==' | '!=') right=expression #EqualityExpr
  | left=expression op=('&&' | '||') right=expression #LogicExpr
  | left=expression '??' right=expression #NullishExpr
  | left=expression 'as' right=type #CastExpr
  | condition=expression '?' value=expression ':' default=expression #TernaryExpr
  | left=expression op=('=' | '+=' | '-=' | '*=' | '/=' | '%=') right=expression #AssignmentExpr
  ;

expressionList : expression (',' expression)* ;

literal
  : INT #IntLiteral
  | FLOAT #FloatLiteral
  | STRING #StringLiteral
  | CHAR #CharLiteral
  | ('true' | 'false') #BoolLiteral
  | '[' array=expressionList? ']' #ArrayLiteral
  | 'null' #NullLiteral
  ;

WS : [ \t\n\r]+ -> skip ;
COMMENT : '/*' .*? '*/' -> skip ;
LINE_COMMENT : '//' .*? ('\r' | '\n' | '\r\n' | EOF) -> skip ;

INT : [0-9]+ ;
FLOAT
  : INT? '.' INT?
  | INT ('.' INT)? (('e' | 'E') '-'? INT)?
  ;
STRING : '"' ( ESC | '\\"' | ~[\\\r\n] )*? '"' { setText(getText().substring(1, getText().length() - 1)); } ;
CHAR : '\'' ( ESC | '\\\'' | ~[\\\r\n'] ) '\'' { setText(getText().substring(1, getText().length() - 1)); } ;

ID : [_a-zA-Z][_a-zA-Z0-9]* ;

fragment ESC : '\\' [btn\\] ;