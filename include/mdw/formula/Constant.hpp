#pragma once
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Grammar.hpp>

namespace mdw { namespace formula {

  template <class T> class ConstExpression: public TypedExpression<T>
  {

  public:

    typedef typename TypeTraits<T>::ReturnType ReturnType;

    ConstExpression(ReturnType iValue, const Grammar& iGrammar):
      TypedExpression<T>(iGrammar), _value(iValue)
    {}

    ConstExpression(ReturnType iValue):
      _value(iValue)
    {}

    ReturnType evaluate(IContext& ioContext) const {
      return _value;
    }

    ReturnType getValue() const
    {
      return _value;
    }

    std::string toString() const
    {
      if (TypedExpression<T>::getType() != kExprString)
      {
        return mdw::lexical_cast<std::string, ReturnType>(_value);
      } else {
        return "'" + mdw::lexical_cast<std::string, ReturnType>(_value) + "'";
      }
    }
  private:
    ReturnType _value;
  };

  template <> class ConstExpression<bool>: public TypedExpression<bool>
  {

  public:

    typedef TypeTraits<bool>::ReturnType ReturnType;

    ConstExpression(ReturnType iValue, const Grammar& iGrammar):
      TypedExpression<bool>(iGrammar), _value(iValue)
    {}

    ConstExpression(ReturnType iValue):
      _value(iValue)
    {}

    ReturnType evaluate(IContext& ioContext) const {
      return _value;
    }

    ReturnType getValue() const
    {
      return _value;
    }

    std::string toString() const
    {
      if (_value)
      {
        return "true";
      } else {
        return "false";
      }
    }

  private:
    ReturnType _value;
  };

}}

