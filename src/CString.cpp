#include <mdw/formula/CString.hpp>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Operator.hpp>
#include <mdw/formula/Repeated.hpp>
#include <mdw/formula/Parser.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Casts.hpp>
#include <mdw/formula/Facts.hpp>
#include <functional>
#include <string>
#include <ctype.h>
#include <boost/mem_fn.hpp>

namespace mdw { namespace formula {

  const char TypeTraits<const char *>::kTypeAsString[] = "CString";

  template <> class ExpressionCast<const char *, int, void>:
    public TypedExpression<int>
    {
      const TypedExpression<const char *>& _expression;
    public:
      typedef TypeTraits<int>::ReturnType ReturnType;

      ExpressionCast(const TypedExpression<const char *>& iExpression,
                     const Grammar& iGrammar,
                     const std::string& iSymbol):
        _expression(iExpression)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        const char *aValue = _expression.evaluate(ioContext);
        if (aValue && isdigit(*aValue))
        {
          return atoi(aValue);
        } else {
          ioContext.setNaN();
          return 0;
        }
      }

      std::string toString() const
      {
        return "(int)(" + _expression.toString() + ")";
      }
    };

  template <> class ExpressionCast<const char *, double, void>:
    public TypedExpression<double>
    {
      const TypedExpression<const char *>& _expression;
    public:
      typedef TypeTraits<double>::ReturnType ReturnType;

      ExpressionCast(const TypedExpression<const char *>& iExpression,
                     const Grammar& iGrammar,
                     const std::string& iSymbol):
        _expression(iExpression)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        const char *aValue = _expression.evaluate(ioContext);
        if (aValue)
        {
          return atof(aValue);
        } else {
          ioContext.setNaN();
          return 0;
        }
      }

      std::string toString() const
      {
        return "(double)(" + _expression.toString() + ")";
      }
    };

  template <> class ExpressionCast<const char *, std::string, void>:
    public TypedExpression<std::string>
    {
      const TypedExpression<const char *>& _expression;
    public:
      typedef TypeTraits<std::string>::ReturnType ReturnType;

      ExpressionCast(const TypedExpression<const char *>& iExpression,
                     const Grammar& iGrammar,
                     const std::string& iSymbol):
        _expression(iExpression)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        const char *aValue = _expression.evaluate(ioContext);
        if (aValue)
        {
          return ioContext.getAllocator().create<std::string>(aValue);
        } else {
          static std::string kEmptyString;
          ioContext.setNaN();
          return kEmptyString;
        }
      }

      std::string toString() const
      {
        return "(string)(" + _expression.toString() + ")";
      }
    };

  class StrLen: public TypedExpression<int>
  {
    const TypedExpression<const char *>& _expression;
  public:
    typedef TypeTraits<int>::ReturnType ReturnType;

    StrLen(const TypedExpression<const char *>& iExpression,
           const Grammar& iGrammar,
           const std::string& iSymbol):
      _expression(iExpression)
    {}

    ReturnType evaluate(IContext& ioContext) const
    {
      const char *aValue = _expression.evaluate(ioContext);
      if (aValue)
      {
        return strlen(aValue);
      } else {
        static std::string kEmptyString;
        ioContext.setNaN();
        return 0;
      }
    }

    std::string toString() const
    {
      return _expression.toString() + ".count";
    }
  };

  class CEmpty: public TypedExpression<bool>
  {
    const TypedExpression<const char *>& _expression;
  public:
    typedef TypeTraits<bool>::ReturnType ReturnType;

    CEmpty(const TypedExpression<const char *>& iExpression,
           const Grammar& iGrammar,
           const std::string& iSymbol):
      _expression(iExpression)
    {}

    ReturnType evaluate(IContext& ioContext) const
    {
      const char *aValue = _expression.evaluate(ioContext);
      return !aValue || (*aValue == 0);
    }

    std::string toString() const
    {
      return _expression.toString() + ".empty";
    }
  };

  Expression& CString::CStringUnary::instantiate(ArenaAllocator& ioAllocator,
                                                   const Grammar& iGrammar,
                                                   const std::string& iSymbol,
                                                   const Expression& iChild) const
  {
    ExpressionType aCStringType = iGrammar.findType<const char *>();
    if (iChild.getType() == aCStringType)
    {
      if (iSymbol == "(int)") {
        return ioAllocator.create<ExpressionCast<const char *, int> >(iChild.get<const char *>(),
                                                                      iGrammar, iSymbol);
      } else if (iSymbol == "(double)") {
        return ioAllocator.create<ExpressionCast<const char *, double> >(iChild.get<const char *>(),
                                                                         iGrammar, iSymbol);
      } else if (iSymbol == "(string)") {
        return ioAllocator.create<ExpressionCast<const char *, std::string> >(iChild.get<const char *>(),
                                                                              iGrammar, iSymbol);
      } else if (iSymbol == "count") {
        return ioAllocator.create<StrLen>(iChild.get<const char *>(), iGrammar, iSymbol);
      } else if (iSymbol == "empty") {
        return ioAllocator.create<CEmpty>(iChild.get<const char *>(), iGrammar, iSymbol);
      } else if (iSymbol == "!") {
        return ioAllocator.create<CEmpty>(iChild.get<const char *>(), iGrammar, iSymbol);
      } else {
        throw mdw::UnknownException("Unary operator not supported on C string: " + iSymbol);
      }
    } else {
      throw mdw::UnknownException("Unsupported unary operator: " + iSymbol + " on type " +
                                  std::string(iChild.getTypeAsString()));
    }
  }

  void CString::CStringUnary::RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator)
  {
    CStringUnary& aUnaryInstantiator = ioAllocator.create<CStringUnary>();
    ExpressionType aCStringType = ioGrammar.registerType<const char *>();
    ioGrammar.registerUnaryOperator(aCStringType, kExprInt, "(int)", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(aCStringType, kExprDouble, "(double)", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(aCStringType, kExprString, "(string)", aUnaryInstantiator);
    ioGrammar.registerAttributeResolver(aCStringType, kExprInt, "count", aUnaryInstantiator);
    ioGrammar.registerAttributeResolver(aCStringType, kExprBool, "empty", aUnaryInstantiator);
    ioGrammar.registerUnaryOperator(aCStringType, kExprBool, "!", aUnaryInstantiator);
  }

  CStringComparator::CStringComparator(const TypedExpression<const char *>& iLeft,
                                       const TypedExpression<const char *>& iRight,
                                       const std::string& iSymbol):
    _left(iLeft), _right(iRight), _symbol(iSymbol)
  {
    _isEqualOK = false;
    _isGreaterOK = false;
    _isLowerOK = false;

    if (iSymbol.find('=') != std::string::npos)
    {
      _isEqualOK = true;
    }
    if (iSymbol.find('>') != std::string::npos)
    {
      _isGreaterOK = true;
    }
    if (iSymbol.find('<') != std::string::npos)
    {
      _isLowerOK = true;
    }
    if (iSymbol == "!=")
    {
      _isEqualOK = false;
      _isLowerOK = true;
      _isGreaterOK = true;
    }
  }

  std::string CStringComparator::toString() const
  {
    return "(" + _left.toString() + ") " + _symbol + " (" + _right.toString() + ")";
  }

  bool CStringComparator::evaluate(IContext& ioContext) const
  {
    const char *aLeft = _left.evaluate(ioContext);
    const char *aRight = _right.evaluate(ioContext);
    if (aLeft == aRight)
    {
      return _isEqualOK;
    } else if ((aLeft == NULL) || (aRight == NULL)) {
      // Only != matches
      return !_isEqualOK && !_isLowerOK && !_isGreaterOK;
    } else {
      int aDiff = strcmp(aLeft, aRight);
      if (aDiff > 0)
      {
        return _isGreaterOK;
      } else if (aDiff < 0) {
        return _isLowerOK;
      } else {
        return _isEqualOK;
      }
    }
  }

  class CStringAt:
    public TypedExpression<__TypeTraits<char>::actual_type>
  {
    const TypedExpression<__TypeTraits<int>::actual_type>& _index;
    const TypedExpression<const char*>& _cstring;

  public:
    CStringAt(const TypedExpression<const char*>& iCString,
              const TypedExpression<__TypeTraits<int>::actual_type>& iIndex):
      _index(iIndex), _cstring(iCString)
    {}

    std::string toString() const
    {
      return "(" + _cstring.toString() + "[" + _index.toString() + "])";
    }

    TypeTraits<char>::ReturnType evaluate(IContext& ioContext) const
    {
      TypeTraits<int>::ReturnType anIndex = _index.evaluate(ioContext);
      const char *aCString = _cstring.evaluate(ioContext);
      if (aCString && (anIndex >= 0) && (anIndex < (TypeTraits<int>::ReturnType)strlen(aCString)))
      {
        return aCString[anIndex];
      } else {
        ioContext.setNaN();
        return '\0';
      }
    }
  };

  StringToCStringComparator::StringToCStringComparator(const TypedExpression<std::string>& iLeft,
                                                       const TypedExpression<const char *>& iRight,
                                                       const std::string& iSymbol,
                                                       bool iRevert):
    _left(iLeft), _right(iRight), _symbol(iSymbol), _revert(iRevert)
  {
    _isEqualOK = false;
    _isGreaterOK = false;
    _isLowerOK = false;

    if (iSymbol.find('=') != std::string::npos)
    {
      _isEqualOK = true;
    }
    if (iSymbol.find('>') != std::string::npos)
    {
      if (iRevert)
      {
        _isLowerOK = true;
      } else {
        _isGreaterOK = true;
      }
    }
    if (iSymbol.find('<') != std::string::npos)
    {
      if (iRevert)
      {
        _isGreaterOK = true;
      } else {
        _isLowerOK = true;
      }
    }
    if (iSymbol == "!=")
    {
      _isEqualOK = false;
      _isLowerOK = true;
      _isGreaterOK = true;
    }
  }

  std::string StringToCStringComparator::toString() const
  {
    if (_revert)
    {
      return "(" + _right.toString() + ") " + _symbol + " (" + _left.toString() + ")";
    } else {
      return "(" + _left.toString() + ") " + _symbol + " (" + _right.toString() + ")";
    }
  }

  bool StringToCStringComparator::evaluate(IContext& ioContext) const
  {
    const std::string& aLeft = _left.evaluate(ioContext);
    const char *aRight = _right.evaluate(ioContext);
    if (aRight == NULL)
    {
      ioContext.setNaN();
      return false;
    } else {
      int aDiff = aLeft.compare(aRight);
      if (aDiff > 0)
      {
        return _isGreaterOK;
      } else if (aDiff < 0) {
        return _isLowerOK;
      } else {
        return _isEqualOK;
      }
    }
  }

  Expression& CString::CStringBinary::instantiate(ArenaAllocator& ioAllocator,
                                                  const Grammar& iGrammar,
                                                  const std::string& iSymbol,
                                                  const Expression& iLeft,
                                                  const Expression& iRight) const
  {
    ExpressionType aCStringType = iGrammar.findType<const char *>();
    if (iLeft.getType() == aCStringType)
    {
      if (iRight.getType() == aCStringType)
      {
        if ((iSymbol == "==") || (iSymbol == "!=") || (iSymbol == ">") || (iSymbol == "<") ||
            (iSymbol == "<=") || (iSymbol == ">="))
        {
          return ioAllocator.create<CStringComparator>(iLeft.get<const char*>(),
                                                       iRight.get<const char*>(),
                                                       iSymbol);
        } else {
          throw mdw::UnknownException("Binary operator not supported on C string: " + iSymbol);
        }
      } else if (iRight.getType() == kExprString) {
        if ((iSymbol == "==") || (iSymbol == "!=") || (iSymbol == ">") || (iSymbol == "<") ||
            (iSymbol == "<=") || (iSymbol == ">="))
        {
          bool aRevert = true;
          return ioAllocator.create<StringToCStringComparator>(iRight.getString(),
                                                               iLeft.get<const char*>(),
                                                               iSymbol, aRevert);
        } else {
          throw mdw::UnknownException("Binary operator not supported on C string: " + iSymbol);
        }
      } else if (iRight.getType() == kExprInt) {
        if (iSymbol == "[]")
        {
          return ioAllocator.create<CStringAt>(iLeft.get<const char *>(), iRight.get<__TypeTraits<int>::actual_type>());
        } else {
          throw mdw::UnknownException("Binary operator not supported on C string: " + iSymbol);
        }
      } else {
        throw mdw::UnknownException("Unsupported binary operator: " + iSymbol + " on types " +
                                    std::string(iLeft.getTypeAsString()) + " and " +
                                    std::string(iRight.getTypeAsString()));
      }
    } else if (iLeft.getType() == kExprString) {
      if (iRight.getType() == aCStringType)
      {
        if ((iSymbol == "==") || (iSymbol == "!=") || (iSymbol == ">") || (iSymbol == "<") ||
            (iSymbol == "<=") || (iSymbol == ">="))
        {
          bool aRevert = false;
          return ioAllocator.create<StringToCStringComparator>(iLeft.getString(),
                                                               iRight.get<const char*>(),
                                                               iSymbol, aRevert);
        } else {
          throw mdw::UnknownException("Binary operator not supported on string: " + iSymbol);
        }
      } else {
        throw mdw::UnknownException("Binary operator not supported on string: " + iSymbol);
      }
    } else {
      throw mdw::UnknownException("Unsupported binary operator: " + iSymbol + " on types " +
                                  std::string(iLeft.getTypeAsString()) + " and " +
                                  std::string(iRight.getTypeAsString()));
    }
  }

  void CString::CStringBinary::RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator)
  {
    CStringBinary& aBinaryInstantiator = ioAllocator.create<CStringBinary>();
    ExpressionType aCStringType = ioGrammar.findType<const char *>();

    ioGrammar.registerBinaryOperator(aCStringType, kExprString, kExprBool, "==",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, kExprString, kExprBool, "!=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, kExprString, kExprBool, ">",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, kExprString, kExprBool, ">=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, kExprString, kExprBool, "<",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, kExprString, kExprBool, "<=",
                                     aBinaryInstantiator);

    ioGrammar.registerBinaryOperator(kExprString, aCStringType, kExprBool, "==",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, aCStringType, kExprBool, "!=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, aCStringType, kExprBool, ">",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, aCStringType, kExprBool, ">=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, aCStringType, kExprBool, "<",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(kExprString, aCStringType, kExprBool, "<=",
                                     aBinaryInstantiator);

    ioGrammar.registerBinaryOperator(aCStringType, aCStringType, kExprBool, "==",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, aCStringType, kExprBool, "!=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, aCStringType, kExprBool, ">",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, aCStringType, kExprBool, ">=",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, aCStringType, kExprBool, "<",
                                     aBinaryInstantiator);
    ioGrammar.registerBinaryOperator(aCStringType, aCStringType, kExprBool, "<=",
                                     aBinaryInstantiator);

    ioGrammar.registerBinaryOperator(aCStringType, kExprInt, kExprInt, "[]", aBinaryInstantiator);
  }

  void CString::RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator)
  {
    ioGrammar.registerType<const char *>();
    CStringUnary::RegisterMe(ioGrammar, ioAllocator);
    CStringBinary::RegisterMe(ioGrammar, ioAllocator);
  }

}}
