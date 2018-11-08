#pragma once

#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Observer.hpp>
#include <mdw/formula/ArenaAllocator.hpp>
#include <mdw/formula/Constant.hpp>
#include <mdw/formula/Facts.hpp>
#include <boost/noncopyable.hpp>
#include <string>
#include <vector>

namespace mdw { namespace formula {

  class Parser: private boost::noncopyable
  {
  public:
    class AdditionalFacts {
      ArenaAllocator& _allocator;
      const Grammar& _grammar;
      std::map<std::string, FactInstantiator*> _facts;
    public:
      AdditionalFacts(ArenaAllocator& ioAllocator, const Grammar& iGrammar);

      void addFact(const std::string& iName, FactInstantiator& iInstantiator);

      template <class BomT> void addDefaultFact(const std::string& iName)
      {
        FactInstantiator& anInstantiator =
          _allocator.create<typename Fact<BomT>::Instantiator>(_grammar);
        addFact(iName, anInstantiator);
      }

      bool hasFact(const std::string& iObject) const;

      void removeFact(const std::string& iName);

      Expression& instantiateFactResolver(const std::string& iObject) const;
    };

    Parser(ArenaAllocator& ioAllocator, const Grammar& iGrammar, const std::string& iFormula,
           AdditionalFacts *ioFacts = NULL);

    Parser(ArenaAllocator& ioAllocator, const Grammar& iGrammar);

    ~Parser();

    void setAdditionalFacts(AdditionalFacts *ioFacts);
    void addObserver(Observer& ioObserver);

    Expression& parse(const std::string& iFormula);

    Expression& getTopExpression();

    const Grammar& getGrammar() const;

    ArenaAllocator& getAllocator();

    /////////////////////////////////////////
    // Callback methods for the YACC parser.
    void setExpression(Expression& ioExpression);

    std::string& createString(const char *iStr, size_t iLen)
    {
      return getAllocator().createString(iStr, iLen);
    }

    template <class T> Expression& createConstant(typename TypeTraits<T>::ReturnType iValue1)
    {
      Expression& aResult = getAllocator().template create<ConstExpression<T> >(iValue1, getGrammar());
      if (_observer)
      {
        return _observer->newConstant(aResult);
      }
      return aResult;
    }

    Expression& createUnaryOperator(Expression& iLeft, const std::string& iSymbol);

    Expression& createBinaryOperator(Expression& iLeft, Expression& iRight,
                                     const std::string& iSymbol);

    Expression& createChoice(Expression& iCondition, Expression& iLeft, Expression& iRight);

    Expression& createCast(Expression& iInput, const char *iOutputType);

    Expression& createFact(const char *iName);

    Expression& createAttribute(Expression& iInput, const char *iName);

    const std::string& getFormula() const;

    // This needs to manage local variables in the Parser itself,
    // and createFact should look up such variables to instantiate default fact getters!
    Expression& createArrowOperator(Expression& iLeft, Expression& iRight, const char *iName);

    void declareLocal(Expression& iLeft, const char *iName);

    void popLocal(const char *iName);


    // The base class is abstract so that new methods HAVE TO be defined in heirs.
    // If you don't need to implement all, though, this class can be inherited from instead.
    class NoopObserver: public Observer {
    public:
      NoopObserver(ArenaAllocator& ioAllocator):
        Observer(ioAllocator)
      {}

      virtual Expression& newFact(Expression& ioResult, const std::string& iName)
      {
        return ioResult;
      }

      virtual Expression& newUnary(Expression& ioResult,
                                   Expression& ioRight,
                                   const std::string& iSymbol)
      {
        return ioResult;
      }

      virtual Expression& newBinary(Expression& ioResult,
                                    Expression& ioLeft,
                                    Expression& ioRight,
                                    const std::string& iSymbol)
      {
        return ioResult;
      }

      virtual Expression& newChoice(Expression& ioResult,
                                    TypedExpression<bool>& ioCondition,
                                    Expression& ioLeft,
                                    Expression& ioRight)
      {
        return ioResult;
      }

      virtual Expression& newArrow(Expression& ioResult,
                                   Expression& ioContainer,
                                   Expression& ioCondition,
                                   const std::string& iLocalName)
      {
        return ioResult;
      }

    };

  private:
    ArenaAllocator& _allocator;
    const Grammar& _grammar;
    const std::string *_formula;
    Expression *_topExpression;
    AdditionalFacts *_additionalFacts;
    Observer *_observer;
    bool _ownedFacts;
  };

}}

