#pragma once

#include <mdw/formula/ArenaAllocator.hpp>
#include <mdw/formula/Observer.hpp>
#include <mdw/formula/Traits.hpp>
#include <set>
#include <list>
#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>

namespace mdw { namespace formula {

  class Expression;
  class KnownType;
  class FactByAddress;
  class Grammar;

  class Factorizer: public Observer, private boost::noncopyable {
    class KnownExpression {
    public:
      explicit KnownExpression(Expression& ioExpression, Factorizer& ioParent);

      void addDependency(const Expression& iExpression);

      Expression& _expression;
      Factorizer& _parent;
      ExpressionType _type;
      size_t _totalComplexity;
      std::set<std::string> _usedFacts;
      std::string _toString;
      Expression *_optimized; // Never NULL, but may change during life-time
    };

  public:

    Factorizer();
    ~Factorizer();

    template <class T> void registerType(const Grammar& iGrammar);

    void reset();

    virtual Expression& newConstant(Expression& ioResult);

    virtual Expression& newFact(Expression& ioResult, const std::string& iName);

    virtual Expression& newUnary(Expression& ioResult,
                                 Expression& ioRight,
                                 const std::string& iSymbol);

    virtual Expression& newBinary(Expression& ioResult,
                                  Expression& ioLeft,
                                  Expression& ioRight,
                                  const std::string& iSymbol);

    virtual Expression& newChoice(Expression& ioResult,
                                  TypedExpression<bool>& ioCondition,
                                  Expression& ioLeft,
                                  Expression& ioRight);

    virtual Expression& newArrow(Expression& ioResult,
                                 Expression& ioContainer,
                                 Expression& ioCondition,
                                 const std::string& iLocalName);

  private:
    KnownExpression *getByDisplay(const std::string& iDisplay);
    KnownExpression *getKnown(const Expression& iExpression);
    KnownExpression& createKnown(Expression& ioExpression, const std::string& iDisplay);
    Expression& optimize(KnownExpression& ioKnown);

    // Ok, a multi_index might be better suited, but hey, the API is too horrible :-D
    // ...and we can afford to copy some pointers.
    // By the way, none of these pointers are ever NULL, but maps don't support references
    typedef boost::unordered_map<const Expression*, KnownExpression*> KnownExpressions;
    typedef boost::unordered_map<std::string, Expression*> ByDisplay;

    typedef boost::unordered_map<std::string, FactByAddress*> Facts;
    // Registered types (fact types + return types)
    typedef boost::unordered_map<ExpressionType, KnownType*> Types;

    ArenaAllocator _allocator;
    KnownExpressions _expressions;
    ByDisplay _displays;
    Facts _facts;
    Types _types;
  };

}}

#include <mdw/formula/impl/Factorizer.hpp>

