#pragma once
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Grammar.hpp>

namespace mdw { namespace formula {

  template <class T>
  ExpressionType BaseTypedExpression<T>::acquireType(const Grammar& iGrammar) const
  {
    return iGrammar.findType<T>();
  }

}}

