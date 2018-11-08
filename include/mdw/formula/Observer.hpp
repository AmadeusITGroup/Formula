#pragma once
#include <string>

namespace mdw { namespace formula {

  class ArenaAllocator;
  class Expression;
  template <class T> class TypedExpression;

  class Observer {
    ArenaAllocator& _allocator;
    Observer *_subObserver;

  public:
    Observer(ArenaAllocator& ioAllocator);
    virtual ~Observer() {}

    void setObserver(Observer *ioSubObserver);
    ArenaAllocator& getAllocator() {
      return _allocator;
    }

    virtual Expression& newConstant(Expression& ioResult) =0;
    virtual Expression& newFact(Expression& ioResult, const std::string& iName) =0;
    virtual Expression& newUnary(Expression& ioResult,
                                 Expression& ioRight,
                                 const std::string& iSymbol) =0;
    virtual Expression& newBinary(Expression& ioResult,
                                  Expression& ioLeft,
                                  Expression& ioRight,
                                  const std::string& iSymbol) =0;
    virtual Expression& newChoice(Expression& ioResult,
                                  TypedExpression<bool>& ioCondition,
                                  Expression& ioLeft,
                                  Expression& ioRight) =0;
    virtual Expression& newArrow(Expression& ioResult,
                                 Expression& ioContainer,
                                 Expression& ioCondition,
                                 const std::string& iLocalName) =0;
  };


}}

