%{
#include <iostream>
#include <stdio.h>
#include <string>
#include <mdw/formula/Parser.hpp>
#include <boost/lexical_cast.hpp>
#include <mdw/formula/Operator.hpp>
#include <mdw/formula/Container.hpp>
#include <mdw/UnknownException.hpp>
#include <boost/regex.hpp>

extern int formulalex(mdw::formula::Parser& ioParser);
extern int formulaparse(mdw::formula::Parser& ioParser);
extern FILE *formulain;

void formulaerror(mdw::formula::Parser&, const char *s);

%}

%locations

%token kGreaterThan
%token kGreaterOrEqual
%token kLowerThan
%token kLowerOrEqual
%token kEqual
%token kNotEqual
%token kPlus
%token kMinus
%token kStar
%token kMatch
%token kModulo
%token kDot
%token kTilda
%token kAssign
%token kQuestion
%token kColon
%token kComma
%token kIn
%token kArrow
%token kPipe
%token kAmpersand

%token kDivided
%token kPower
%token kLeftSquare;
%token kRightSquare;
%token kLeftParenthesis;
%token kRightParenthesis;
%token kAnd;
%token kOr;
%token kNot;
%token kTrue;
%token kFalse;

%token <identifier> kFact;
%token <identifier> kIdentifier;

%token <floatValue> kFloat
%token <intValue> kInteger
%token <currentString> kString

%type <expression> expr
%type <expression> local_var
%type <expression> constant
%type <expression> included
%type <expression> logical_and
%type <expression> comparison
%type <expression> in
%type <expression> term
%type <expression> factor
%type <expression> logical
%type <expression> unary
%type <expression> terminal
%type <expression> power
%type <expression> condition

/*%type <expression> exprList*/

/*%left kAnd kOr*/
/*%left kLowerThan kLowerOrEqual kGreaterThan kGreaterOrEqual kEqual kNotEqual*/
/*%left kPlus kMinus*/
/*%left kStar kDivided*/
/*%left NEG*/     /* negation--unary minus */
/*%right kPower*/
/*%left kNot*/


%parse-param {mdw::formula::Parser& ioParser}
%lex-param { mdw::formula::Parser& ioParser }

%start program
%%

%union
{
    std::string    *currentString;
    mdw::formula::Expression *expression;
    char   identifier[101];
    double  floatValue;
    mdw::formula::Expression *constant;
    mdw::formula::Expression *included;
    bool    boolValue;
    int64_t intValue;

};

program : expr {
        ioParser.setExpression(*$1);
        }

expr    :  local_var { $$ = $1; } 
        |  terminal kAssign condition {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, ":=");
        } 
        |  condition { $$ = $1; }
        ;

local_var: condition kArrow kIdentifier {
         ioParser.declareLocal(*$1, $3);
         } kQuestion expr {
         $$ = &ioParser.createArrowOperator(*$1, *$6, $3);
         ioParser.popLocal($3);
         }
condition:  logical {$$ = $1;}
        |  logical kQuestion logical kColon logical {
          $$ = &ioParser.createChoice(*$1, *$3, *$5);
        }
        ;

logical : logical_and {$$ = $1;}
        | logical kOr logical_and {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "||");
        } 
        ;

logical_and: comparison {$$ = $1;}
           | logical_and kAnd comparison {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "&&");
        }
           ;

comparison : in  {$$ = $1;}
        | in kGreaterOrEqual in {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, ">=");
        } 
        | in kGreaterThan in {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, ">");
        } 
        | in kLowerOrEqual in {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "<=");
        } 
        | in kLowerThan in {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "<");
        } 
        | in kMatch in {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "~=");
        } 
        | in kEqual in {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "==");
        } 
        | in kNotEqual in {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "!=");
        } 
        ;

in      : term {$$ = $1;}
           | term kIn term {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "in");
        }
           ;

term    : factor {$$ = $1;}
        | term kPlus factor {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "+");
        }
        | term kMinus factor {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "-");
        }
        | term kPipe factor {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "|");
        }
        ;

factor  : power {$$ = $1;}
        | factor kStar power {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "*");
        }
        | factor kDivided power {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "/");
        }
        | factor kModulo power {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "%");
        }
        | factor kAmpersand power {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "&");
        }
        ;

power   : unary {$$ = $1;}
        | unary kPower power {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "^");
        }
        ;

unary   : kNot unary {
        $$ = &ioParser.createUnaryOperator(*$2, "!");
        }
        | kMinus unary {
        $$ = &ioParser.createUnaryOperator(*$2, "-");
        }
        | kPlus unary {
        $$ = &ioParser.createUnaryOperator(*$2, "+");
        }
        | kStar unary {
        $$ = &ioParser.createUnaryOperator(*$2, "*");
        }
        | kLeftParenthesis kIdentifier kRightParenthesis unary {
        $$ = &ioParser.createCast(*$4, $2);
        }
        | terminal {$$ = $1;}
        ;

terminal:  constant {$$ = $1;}
        |  kFact { $$ = &ioParser.createFact($1); }
        |  terminal kDot kIdentifier { $$ = &ioParser.createAttribute(*$1, $3); }
        |  included {$$ = $1;}
        |  terminal kLeftSquare expr kRightSquare {
        $$ = &ioParser.createBinaryOperator(*$1, *$3, "[]");
        }
     /* | terminal kDot kIdentifier kLeftParenthesis exprList kRightParenthesis */
        ;

constant:  kTrue  {
                  $$ = &ioParser.createConstant<bool>(true);
                  }
         | kFalse {
                  $$ = &ioParser.createConstant<bool>(false);
                  }
         | kInteger {
                  $$ = &ioParser.createConstant<int64_t>($1);
                  }
         | kFloat {
                  $$ = &ioParser.createConstant<double>($1);
                  }
         | kString{
                  $$ = &ioParser.createConstant<std::string>(*$1);
                  }
                  ;
        
/*exprList: 
        | expr
        | exprList kComma expr
        ;*/

included: kLeftParenthesis expr kRightParenthesis {
        $$ = $2;
        };
%%

void formulaerror (mdw::formula::Parser& ioParser, char const *s) {
  throw mdw::UnknownException("Invalid syntax in the formula: \"" + ioParser.getFormula() + "\"");
 }

