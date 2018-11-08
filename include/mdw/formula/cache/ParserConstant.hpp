#pragma once
#include <mdw/formula/Expression.hpp>

namespace mdw { namespace formula {

  template <class T> class ParserConstant: public TypedExpression<T>
  {

  public:

    typedef typename TypeTraits<T>::ReturnType ReturnType;

    ParserConstant(ReturnType iValue, ExpressionType iType, const Expression& iInitial):
      TypedExpression<T>(iType), _value(iValue), _initialExpression(iInitial)
    {}

    ReturnType evaluate(IContext& ioContext) const {
      return _value;
    }

    std::string toString() const
    {
      return _initialExpression.toString();
    }
  private:
    ReturnType _value;
    const Expression& _initialExpression;
  };

}}

