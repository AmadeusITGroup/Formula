#pragma once

#include <functional>
#include <mdw/formula/ArenaAllocator.hpp>
#include <mdw/formula/Traits.hpp>
#include <mdw/Tracer.hpp>
#include <boost/unordered_map.hpp>

namespace mdw { namespace formula {

  class Expression;
  class IContext;
  
  class FactByAddress
  {
  public:
    virtual const void *compute(IContext& ioContext) const = 0;
    virtual const Expression& getExpression() const = 0;
  };

  class KnownType {
    const ExpressionType _type;
    const std::string _asString;

  public:
    KnownType(ExpressionType iType, const std::string& iAsString):
      _type(iType), _asString(iAsString)
    {}

    virtual ~KnownType() {}

    ExpressionType getType() const {
      return _type;
    }

    virtual Expression& getUnaryCached(Expression& ioChild,
                                       const FactByAddress& ioFact,
                                       ArenaAllocator& ioAllocator) const = 0;

    virtual Expression& getConstant(Expression& ioInitial,
                                    ExpressionType iType,
                                    ArenaAllocator& ioAllocator) const = 0;

    virtual FactByAddress& getFactByAddress(const Expression& iFact,
                                            ArenaAllocator& ioAllocator) const = 0;
  };

}}
