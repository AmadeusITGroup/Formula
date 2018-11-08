#pragma once

#include <functional>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/cache/KnownType.hpp>
#include <mdw/formula/cache/CachedExpression.hpp>
#include <mdw/formula/cache/ParserConstant.hpp>
#include <mdw/Tracer.hpp>
#include <boost/unordered_map.hpp>

namespace mdw { namespace formula {

  template <class T>
    Expression& ActualType<T>::getUnaryCached(Expression& ioChild,
                                              const FactByAddress& ioFact,
                                              ArenaAllocator& ioAllocator) const
    {
      ExpressionType aType = getType();
      return ioAllocator.create<typename UnaryCachedByAddress<T>::Expression>(aType,
                                                                              ioChild.get<T>(),
                                                                              ioFact);
    }

  template <class T>
    Expression& ActualType<T>::getConstant(Expression& ioInitial,
                                           ExpressionType iType,
                                           ArenaAllocator& ioAllocator) const
    {
      try {
        // Allocate "temporary" results into the final allocator
        IContext aContext(ioAllocator);
        typename TypeTraits<T>::ReturnType aValue = ioInitial.get<T>().evaluate(aContext);
        return ioAllocator.create<ParserConstant<T> >(aValue, iType, ioInitial);
      } catch (...) {
        FORMULA_DEBUG("Could not get value of constant: " << ioInitial.toString());
        return ioInitial;
      }
    }

  template <class T>
    FactByAddress& ActualType<T>::getFactByAddress(const Expression& iFact,
                                                   ArenaAllocator& ioAllocator) const
    {
      return ioAllocator.create<ActualFactByAddress<T> >(iFact);
    }

  template <class T>
    ActualFactByAddress<T>::ActualFactByAddress(const Expression& iExpression):
      _expression(iExpression.get<T>())
  {}

  template <class T>
    const void *ActualFactByAddress<T>::compute(IContext& ioContext) const
    {
      return __TypeTraits<typename TypeTraits<T>::ReturnType>::ToVoid(_expression.evaluate(ioContext));
    }

  template <class T> const Expression& ActualFactByAddress<T>::getExpression() const
  {
    return _expression;
  }

}}
