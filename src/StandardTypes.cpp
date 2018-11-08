#include <mdw/formula/StandardTypes.hpp>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Operator.hpp>
#include <mdw/formula/Repeated.hpp>
#include <mdw/formula/Parser.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Casts.hpp>
#include <mdw/formula/Facts.hpp>
#include <mdw/formula/ValueException.hpp>
#include <functional>
#include <string>
#include <boost/mem_fn.hpp>


namespace mdw { namespace formula {

  class LogicalOrOperator :
    public SymmetricTypedOperator<bool, bool, std::logical_or<bool> >
  {
    public:
      LogicalOrOperator(const TypedExpression<bool>& iLeft,
                        const TypedExpression<bool>& iRight,
                        const Grammar& iGrammar,
                        const std::string& iSymbol) :
        SymmetricTypedOperator<bool, bool, std::logical_or<bool> >(iLeft, iRight, iGrammar, iSymbol)
      {}

      // PTR#12761056
      // Let's first evaluate the left operand on his own : the goal is to handle the cases where
      // the left operand throws a ValueException or flags the context as NaN in which cases,
      // since we are in an OR statement, we should ignore the exception / rollback the NaN flag
      // not to invalidate the right operand evaluation upfront
      SymmetricTypedOperator<bool, bool, std::logical_or<bool> >::ReturnType evaluate(
          IContext& ioContext) const
      {
        // Better safe than sorry
        if(ioContext.isNaN())
        {
          return false;
        }

        bool aLeftEvaluate;
        try
        {
          aLeftEvaluate = _left.evaluate(ioContext);
        }
        catch(const ValueException& aValueException)
        {
          aLeftEvaluate = false;
        }

        if(ioContext.isNaN())
        {
          ioContext.ignoreNaN();
          aLeftEvaluate = false;
        }

        return aLeftEvaluate || _right.evaluate(ioContext);
      }
  };

  Expression& StandardUnary::instantiate(ArenaAllocator& ioAllocator,
                                         const Grammar& iGrammar,
                                         const std::string& iSymbol,
                                         const Expression& iChild) const
  {
    switch (iChild.getType()) {
    case kExprInt:
      {
        if (iSymbol == "-")
        {
          return ioAllocator.create<TypedUnaryOperator<int, int, std::negate<int64_t> > >(iChild.getInt(), iGrammar, iSymbol);
        } else if (iSymbol == "(double)") {
          return ioAllocator.create<ExpressionCast<int, double> >(iChild.getInt(), iGrammar, iSymbol);
        } else if (iSymbol == "(bool)") {
          return ioAllocator.create<ExpressionCast<int, bool> >(iChild.getInt(), iGrammar, iSymbol);
        } else if (iSymbol == "(string)") {
          return ioAllocator.create<ExpressionCast<int, std::string> >(iChild.getInt(), iGrammar, iSymbol);
        } else {
          throw mdw::UnknownException("Unary operator not supported on int: " + iSymbol);
        }
        break;
      }
    case kExprDouble:
      {
        if (iSymbol == "-")
        {
          return ioAllocator.create<TypedUnaryOperator<double, double, std::negate<double> > >(iChild.getDouble(), iGrammar, iSymbol);
        } else if (iSymbol == "(int)") {
          return ioAllocator.create<ExpressionCast<double, int> >(iChild.getDouble(), iGrammar, iSymbol);
        } else if (iSymbol == "(string)") {
          return ioAllocator.create<ExpressionCast<double, std::string> >(iChild.getDouble(), iGrammar, iSymbol);
        } else {
          throw mdw::UnknownException("Unary operator not supported on double: " + iSymbol);
        }
        break;
      }
    case kExprBool:
      {
        if (iSymbol == "!") {
          return ioAllocator.create<TypedUnaryOperator<bool, bool, std::logical_not<bool> > >(iChild.getBool(), iGrammar, iSymbol);
        } else if (iSymbol == "(int)") {
          return ioAllocator.create<ExpressionCast<bool, int> >(iChild.getBool(), iGrammar, iSymbol);
        } else {
          throw mdw::UnknownException("Unary operator not supported on bool: " + iSymbol);
        }
        break;
      }
    case kExprString:
      {
        if (iSymbol == "(int)") {
          return ioAllocator.create<ExpressionCast<std::string, int> >(iChild.getString(), iGrammar, iSymbol);
        } else if (iSymbol == "(double)") {
          return ioAllocator.create<ExpressionCast<std::string, double> >(iChild.getString(), iGrammar, iSymbol);
        } else {
          throw mdw::UnknownException("Unary operator not supported on string: " + iSymbol);
        }
        break;
      }
    default:
      throw mdw::UnknownException("Unsupported unary operator: " + iSymbol + " on type " +
                                  std::string(iChild.getTypeAsString()));
    }
  }

  void StandardUnary::RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator)
  {
    StandardUnary& aUnaryInstantiator = ioAllocator.create<StandardUnary>();
    ioGrammar.registerUnaryOperator(kExprInt, kExprInt, "-", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(kExprDouble, kExprDouble, "-", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(kExprBool, kExprBool, "!", aUnaryInstantiator);

    ioGrammar.registerUnaryOperator(kExprInt, kExprDouble, "(double)", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(kExprInt, kExprBool, "(bool)", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(kExprInt, kExprString, "(string)", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(kExprDouble, kExprInt, "(int)", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(kExprDouble, kExprString, "(string)", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(kExprBool, kExprInt, "(int)", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(kExprString, kExprInt, "(int)", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(kExprString, kExprDouble, "(double)", aUnaryInstantiator);
  }

  Expression& StandardBinary::instantiate(ArenaAllocator& ioAllocator,
                                          const Grammar& iGrammar,
                                          const std::string& iSymbol,
                                          const Expression& iLeft,
                                          const Expression& iRight) const
  {
    switch (iLeft.getType()) {
    case kExprInt:
      {
        if (iRight.getType() == kExprInt)
        {
          if (iSymbol == "+")
          {
            return ioAllocator.create<SymmetricTypedOperator<int, int, std::plus<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == "-") {
            return ioAllocator.create<SymmetricTypedOperator<int, int, std::minus<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == "*") {
            return ioAllocator.create<SymmetricTypedOperator<int, int, std::multiplies<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == "/") {
            return ioAllocator.create<SymmetricTypedOperator<int, int, std::divides<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == "%") {
            return ioAllocator.create<SymmetricTypedOperator<int, int, std::modulus<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == ">") {
            return ioAllocator.create<SymmetricTypedOperator<int, bool, std::greater<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == ">=") {
            return ioAllocator.create<SymmetricTypedOperator<int, bool, std::greater_equal<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == "<") {
            return ioAllocator.create<SymmetricTypedOperator<int, bool, std::less<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == "<=") {
            return ioAllocator.create<SymmetricTypedOperator<int, bool, std::less_equal<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == "!=") {
            return ioAllocator.create<SymmetricTypedOperator<int, bool, std::not_equal_to<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else if (iSymbol == "==") {
            return ioAllocator.create<SymmetricTypedOperator<int, bool, std::equal_to<int64_t> > >(iLeft.getInt(), iRight.getInt(), iGrammar, iSymbol);
          } else {
            throw mdw::UnknownException("Binary operator not supported on int: " + iSymbol);
          }
        }
        break;
      }
    case kExprDouble:
      {
        if (iRight.getType() == kExprDouble)
        {
          if (iSymbol == "+")
          {
            return ioAllocator.create<SymmetricTypedOperator<double, double, std::plus<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else if (iSymbol == "-") {
            return ioAllocator.create<SymmetricTypedOperator<double, double, std::minus<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else if (iSymbol == "*") {
            return ioAllocator.create<SymmetricTypedOperator<double, double, std::multiplies<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else if (iSymbol == "/") {
            return ioAllocator.create<SymmetricTypedOperator<double, double, std::divides<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else if (iSymbol == ">") {
            return ioAllocator.create<SymmetricTypedOperator<double, bool, std::greater<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else if (iSymbol == ">=") {
            return ioAllocator.create<SymmetricTypedOperator<double, bool, std::greater_equal<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else if (iSymbol == "<") {
            return ioAllocator.create<SymmetricTypedOperator<double, bool, std::less<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else if (iSymbol == "<=") {
            return ioAllocator.create<SymmetricTypedOperator<double, bool, std::less_equal<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else if (iSymbol == "!=") {
            return ioAllocator.create<SymmetricTypedOperator<double, bool, std::not_equal_to<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else if (iSymbol == "==") {
            return ioAllocator.create<SymmetricTypedOperator<double, bool, std::equal_to<double> > >(iLeft.getDouble(), iRight.getDouble(), iGrammar, iSymbol);
          } else {
            throw mdw::UnknownException("Binary operator not supported on double: " + iSymbol);
          }
        }
        break;
      }
    case kExprBool:
      {
        if (iRight.getType() == kExprBool)
        {
          if (iSymbol == "&&")
          {
            return ioAllocator.create<SymmetricTypedOperator<bool, bool, std::logical_and<bool> > >(iLeft.getBool(), iRight.getBool(), iGrammar, iSymbol);
          } else if (iSymbol == "||") {
            return ioAllocator.create<LogicalOrOperator>(iLeft.getBool(), iRight.getBool(), iGrammar, iSymbol);
          } else if (iSymbol == "==") {
            return ioAllocator.create<SymmetricTypedOperator<bool, bool, std::equal_to<bool> > >(iLeft.getBool(), iRight.getBool(), iGrammar, iSymbol);
          } else if (iSymbol == "!=") {
            return ioAllocator.create<SymmetricTypedOperator<bool, bool, std::not_equal_to<bool> > >(iLeft.getBool(), iRight.getBool(), iGrammar, iSymbol);
          } else {
            throw mdw::UnknownException("Binary operator not supported on bool: " + iSymbol);
          }
        }
        break;
      }
    case kExprString:
      {
        if (iRight.getType() == kExprString)
        {
          if (iSymbol == "==")
          {
            return ioAllocator.create<SymmetricTypedOperator<std::string, bool, std::equal_to<std::string> > >(iLeft.getString(), iRight.getString(), iGrammar, iSymbol);
          } else if (iSymbol == "!=") {
            return ioAllocator.create<SymmetricTypedOperator<std::string, bool, std::not_equal_to<std::string> > >(iLeft.getString(), iRight.getString(), iGrammar, iSymbol);
          } else if (iSymbol == ">") {
            return ioAllocator.create<SymmetricTypedOperator<std::string, bool, std::greater<std::string> > >(iLeft.getString(), iRight.getString(), iGrammar, iSymbol);
          } else if (iSymbol == ">=") {
            return ioAllocator.create<SymmetricTypedOperator<std::string, bool, std::greater_equal<std::string> > >(iLeft.getString(), iRight.getString(), iGrammar, iSymbol);
          } else if (iSymbol == "<") {
            return ioAllocator.create<SymmetricTypedOperator<std::string, bool, std::less<std::string> > >(iLeft.getString(), iRight.getString(), iGrammar, iSymbol);
          } else if (iSymbol == "<=") {
            return ioAllocator.create<SymmetricTypedOperator<std::string, bool, std::less_equal<std::string> > >(iLeft.getString(), iRight.getString(), iGrammar, iSymbol);
          } else {
            throw mdw::UnknownException("Binary operator not supported on string: " + iSymbol);
          }
        } else if (iRight.getType() == kExprInt) {
          if (iSymbol == "[]")
          {
            SquareOp<size_t, std::string> anOp;
            return ioAllocator.create<SquareOperator<SquareOp<size_t, std::string> > >(iLeft.getString(),
                                                                                    iRight.getLong(),
                                                                                    iGrammar,
                                                                                    anOp);
          } else {
            throw mdw::UnknownException("Binary operator not supported on string: " + iSymbol);
          }
          break;
        }
      }
    default:
      throw mdw::UnknownException("Unsupported binary operator: " + iSymbol + " on types " +
                                  std::string(iLeft.getTypeAsString()) + " and " +
                                  std::string(iRight.getTypeAsString()));
    }
    throw mdw::UnknownException("Unsupported binary operator: " + iSymbol + " on types " +
                                std::string(iLeft.getTypeAsString()) + " and " +
                                std::string(iRight.getTypeAsString()));
  }

  void StandardBinary::RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator)
  {
    StandardBinary& aBinaryInstantiator = ioAllocator.create<StandardBinary>();
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprInt, "+", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprInt, "-", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprInt, "*", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprInt, "/", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprInt, "%", aBinaryInstantiator);

    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprBool, ">", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprBool, ">=", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprBool, "<", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprBool, "<=", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprBool, "!=", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprInt, kExprInt, kExprBool, "==", aBinaryInstantiator);

    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprDouble, "+",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprDouble, "-",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprDouble, "*",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprDouble, "/",
                                     aBinaryInstantiator);

    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprBool, ">", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprBool, ">=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprBool, "<", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprBool, "<=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprBool, "!=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprDouble, kExprDouble, kExprBool, "==",
                                     aBinaryInstantiator);

    ioGrammar.registerBinaryOperator(kExprBool, kExprBool, kExprBool, "&&", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprBool, kExprBool, kExprBool, "||", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprBool, kExprBool, kExprBool, "==", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprBool, kExprBool, kExprBool, "!=", aBinaryInstantiator);

    ioGrammar.registerBinaryOperator(kExprString, kExprString, kExprBool, "==",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, kExprString, kExprBool, "!=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, kExprString, kExprBool, ">", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, kExprString, kExprBool, ">=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, kExprString, kExprBool, "<", aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, kExprString, kExprBool, "<=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, kExprInt, kExprInt, "[]", aBinaryInstantiator);
  }

}}
