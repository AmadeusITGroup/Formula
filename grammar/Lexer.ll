%option noyywrap nounput nounistd
%option yylineno

%{

extern int formulalex();

#include <iostream>
#include <stdio.h>
#include <mdw/formula/Parser.hpp>
#include <sstream>
#include <boost/lexical_cast.hpp>

#define YY_DECL int formulalex(mdw::formula::Parser& ioParser)

%}

GREATER_THAN         ">"
GREATER_OR_EQUAL     ">="
LOWER_THAN           "<"
LOWER_OR_EQUAL       "<="
EQUAL                "=="
MATCH                "~="
NOT_EQUAL            "!="|"<>"|"/="
PLUS                 "+"
MINUS                "-"
STAR                 "*"
DIVIDED              "/"
POWER                "^"
MODULO               "%"
LEFT_PARENTHESIS     "("
RIGHT_PARENTHESIS    ")"
LEFT_SQUARE          "["
RIGHT_SQUARE         "]"
PIPE                 "|"
AMPERSAND            "&"
DOT                  "."
QUESTION             "?"
COLON                ":"
COMMA                ","
TILDA                "~"
ARROW                "->"
ASSIGN               ":="
AND                  "AND"|"and"|"And"|"&&"
OR                   "OR"|"or"|"Or"|"||"
NOT                  "NOT"|"not"|"Not"|"!"
TRUE                 "true"|"TRUE"|"YES"|"yes"|"True"|"Yes"
FALSE                "false"|"FALSE"|"NO"|"no"|"False"|"No"
IN                   "IN"|"in"|"In"
OBJECT_NAME          \$([A-Za-z][A-Za-z0-9_]*)
IDENTIFIER           ([A-Za-z_][A-Za-z0-9_]*)
STRING               \"(\\.|[^"])*\"|\'(\\.|[^'])*\'
FLOAT                ([0-9]*\.[0-9]+|[0-9]+\.)
INTEGER              (0|[1-9][0-9]*)


%%

{GREATER_THAN}                                  { return kGreaterThan; }

{GREATER_OR_EQUAL}                              { return kGreaterOrEqual; }

{LOWER_THAN}                                    { return kLowerThan; }

{LOWER_OR_EQUAL}                                { return kLowerOrEqual; }

{EQUAL}                                         { return kEqual; }

{NOT_EQUAL}                                     { return kNotEqual; }

{PLUS}                                          { return kPlus; }

{MINUS}                                         { return kMinus; }

{STAR}                                          { return kStar; }

{DIVIDED}                                       { return kDivided; }

{POWER}                                         { return kPower; }

{IN}                                            { return kIn; }

{ASSIGN}                                        { return kAssign; }

{TILDA}                                         { return kTilda; }

{QUESTION}                                      { return kQuestion; }

{COLON}                                         { return kColon; }

{ARROW}                                         { return kArrow; }

{MODULO}                                        { return kModulo; }

{COMMA}                                         { return kComma; }

{PIPE}                                          { return kPipe; }

{AMPERSAND}                                     { return kAmpersand; }

{DOT}                                           { return kDot; }

{MATCH}                                         { return kMatch; }

{LEFT_SQUARE}                                   { return kLeftSquare; }

{RIGHT_SQUARE}                                  { return kRightSquare; }

{LEFT_PARENTHESIS}                              { return kLeftParenthesis; }

{RIGHT_PARENTHESIS}                             { return kRightParenthesis; }

{AND}                                           { return kAnd;}

{OR}                                            { return kOr;}

{NOT}                                           { return kNot;}

{TRUE}                                          { return kTrue;}

{FALSE}                                         { return kFalse;}

{FLOAT} {
  formulalval.floatValue = atof(formulatext);
  return kFloat;
}

{INTEGER} {
  formulalval.intValue = atoi(formulatext);
  return kInteger;
}

{STRING} {
  formulalval.currentString = &ioParser.createString(formulatext + 1, formulaleng - 2);
  return kString;
}

{OBJECT_NAME} {
  if (formulaleng > 101) {
    throw mdw::UnknownException("Variable name is longer than 100 characters - not supported");
  }
  // Skip the initial $ sign
  memcpy(formulalval.identifier, formulatext + 1, formulaleng - 1);
  formulalval.identifier[formulaleng - 1] = 0;
  return kFact;
}

{IDENTIFIER} {
  if (formulaleng > 100) {
    throw mdw::UnknownException("Identifier is longer than 100 characters - not supported");
  }
  memcpy(formulalval.identifier, formulatext, formulaleng);
  formulalval.identifier[formulaleng] = 0;
  return kIdentifier;
}

[\t]+                                           /* ignore */
[\n]+                                           /* ignore */
[\r]+                                           /* ignore */
[[:space:]]+                                    /* ignore */

%%
