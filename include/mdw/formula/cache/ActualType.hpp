#pragma once

#include <mdw/formula/cache/KnownType.hpp>
#include <mdw/Tracer.hpp>
#include <boost/unordered_map.hpp>

namespace mdw { namespace formula {

  template <class T> class TypedExpression;

  template <class T> class ActualType: public KnownType
  {
  public:
    ActualType(ExpressionType iType):
      KnownType(iType, TypeTraits<T>::kTypeAsString)
    {}

    Expression& getUnaryCached(Expression& ioChild,
                               const FactByAddress& ioFact,
                               ArenaAllocator& ioAllocator) const;

    Expression& getConstant(Expression& ioInitial,
                            ExpressionType iType,
                            ArenaAllocator& ioAllocator) const;

    virtual FactByAddress& getFactByAddress(const Expression& iFact, ArenaAllocator& ioAllocator) const;
  };

  template <class T> class ActualFactByAddress: public FactByAddress
  {
    const TypedExpression<T>& _expression;
  public:
    ActualFactByAddress(const Expression& iExpression);

    virtual const void *compute(IContext& ioContext) const;
    virtual const Expression& getExpression() const;
  };

}}

#include <mdw/formula/impl/ActualType.hpp>

