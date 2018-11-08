#include <mdw/formula/Parser.hpp>
#include <mdw/formula/Observer.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Operator.hpp>

#include <mdw/formula/parse/yacc/YaccParser.hpp>
#include <mdw/formula/parse/flex/Lexer.hpp>

#include <boost/foreach.hpp>
#include <mdw/Tracer.hpp>

extern int formulaparse(mdw::formula::Parser& ioParser);

namespace mdw { namespace formula {

  Parser::AdditionalFacts::AdditionalFacts(ArenaAllocator& ioAllocator, const Grammar& iGrammar):
    _allocator(ioAllocator), _grammar(iGrammar)
  {}

  void Parser::AdditionalFacts::addFact(const std::string& iName, FactInstantiator& iInstantiator)
  {
    _facts[iName] = &iInstantiator;
  }

  void Parser::AdditionalFacts::removeFact(const std::string& iName)
  {
    _facts.erase(iName);
  }

  bool Parser::AdditionalFacts::hasFact(const std::string& iObject) const
  {
    return _facts.find(iObject) != _facts.end();
  }

  Expression& Parser::AdditionalFacts::instantiateFactResolver(const std::string& iObject) const
  {
    std::map<std::string, FactInstantiator*>::const_iterator anIt = _facts.find(iObject);
    if (anIt != _facts.end() && anIt->second != NULL)
    {
      return anIt->second->instantiate(_allocator, _grammar, iObject);
    } else {
      return _grammar.instantiateFactResolver(_allocator, iObject);
    }
  }

  Parser::Parser(ArenaAllocator& ioAllocator, const Grammar& iGrammar,
                 const std::string& iFormula, AdditionalFacts *ioFacts):
    _allocator(ioAllocator), _grammar(iGrammar), _formula(&iFormula),
    _topExpression(NULL), _additionalFacts(ioFacts), _observer(NULL), _ownedFacts(false)
  {
    parse(iFormula);
  }

  Parser::Parser(ArenaAllocator& ioAllocator, const Grammar& iGrammar):
    _allocator(ioAllocator), _grammar(iGrammar), _formula(NULL),
    _topExpression(NULL), _additionalFacts(NULL), _observer(NULL), _ownedFacts(false)
  {}

  Parser::~Parser()
  {
    if (_ownedFacts && _additionalFacts)
    {
      delete _additionalFacts;
      _additionalFacts = NULL;
    }
  }

  void Parser::setAdditionalFacts(AdditionalFacts *ioFacts)
  {
    if (_ownedFacts)
    {
      delete _additionalFacts;
    }
    _additionalFacts = ioFacts;
    _ownedFacts = false;
  }

  void Parser::addObserver(Observer& ioObserver)
  {
    ioObserver.setObserver(_observer);
    _observer = &ioObserver;
  }

  Expression& Parser::parse(const std::string& iFormula)
  {
    _formula = &iFormula;

    formula_scan_string(iFormula.c_str());

    FORMULA_DEBUG("Parsing formula: " << iFormula);

    formulaparse(*this);
    formulalex_destroy();
    formula_delete_buffer(YY_CURRENT_BUFFER);
    return getTopExpression();
  }

  Expression& Parser::getTopExpression()
  {
    if (_topExpression)
    {
      FORMULA_DEBUG("Expression is now @ " << _topExpression);
      return *_topExpression;
    } else {
      throw mdw::UnknownException("No expression found in formula");
    }
  }

  void Parser::setExpression(Expression& ioExpression)
  {
    FORMULA_DEBUG("Saving expression @" << &ioExpression);
    _topExpression = &ioExpression;
  }

  const Grammar& Parser::getGrammar() const
  {
    return _grammar;
  }

  const std::string& Parser::getFormula() const
  {
    if (_formula)
    {
      return *_formula;
    } else {
      throw mdw::UnknownException("No formula entered in the parser");
    }
  }

  ArenaAllocator& Parser::getAllocator()
  {
    return _allocator;
  }

  Expression& Parser::createUnaryOperator(Expression& iLeft, const std::string& iSymbol)
  {
    Expression& aResult = getGrammar().instantiateUnaryOperator(getAllocator(), iLeft, iSymbol);
    if (_observer)
    {
      return _observer->newUnary(aResult, iLeft, iSymbol);
    } else {
      return aResult;
    }
  }

  Expression& Parser::createBinaryOperator(Expression& iLeft, Expression& iRight,
                                           const std::string& iSymbol)
  {
    Expression& aResult =
      getGrammar().instantiateBinaryOperator(getAllocator(), iLeft, iRight, iSymbol);
    if (_observer)
    {
      return _observer->newBinary(aResult, iLeft, iRight, iSymbol);
    } else {
      return aResult;
    }
  }

  Expression& Parser::createArrowOperator(Expression& iLeft, Expression& iRight, const char *iName)
  {
    Expression& aResult =
      getGrammar().instantiateArrowOperator(getAllocator(), iLeft, iRight, iName);
    if (_observer)
    {
      return _observer->newArrow(aResult, iLeft, iRight, iName);
    } else {
      return aResult;
    }
  }

  void Parser::declareLocal(Expression& iLeft, const char *iName)
  {
    FactInstantiator *aTemporary = getGrammar().instantiateLocal(getAllocator(), iLeft, iName);
    if (aTemporary)
    {
      if (!_additionalFacts)
      {
        _additionalFacts = new AdditionalFacts(getAllocator(), getGrammar());
        _ownedFacts = true;
      }
      if (_additionalFacts->hasFact(iName))
      {
        throw mdw::UnknownException("Temporary variable overloads itself: " + std::string(iName));
      }
      _additionalFacts->addFact(iName, *aTemporary);
    }
  }

  void Parser::popLocal(const char *iName)
  {
    if (!_additionalFacts || !_additionalFacts->hasFact(iName))
    {
      throw mdw::UnknownException("Cannot remove missing fact: " + std::string(iName));
    }
    _additionalFacts->removeFact(iName);
  }

  Expression& Parser::createChoice(Expression& iCondition, Expression& iLeft, Expression& iRight)
  {
    if (iCondition.getType() != kExprBool)
    {
      throw mdw::UnknownException("Choice condition " + iCondition.toString() + " must be boolean!");
    }
    if (iLeft.getType() == iRight.getType())
    {
      TypedExpression<bool>& aCondition = iCondition.getBool();
      Expression& aResult =
        iLeft.createChoice(getAllocator(), getGrammar(), aCondition, iLeft, iRight);
      if (_observer)
      {
        return _observer->newChoice(aResult, aCondition, iLeft, iRight);
      } else {
        return aResult;
      }
    } else {
      throw mdw::UnknownException("Choice expressions are of different types: " +
                                  iLeft.getTypeAsString() + " vs. " + iRight.getTypeAsString());
    }
  }

  Expression& Parser::createCast(Expression& iLeft, const char *iOutputType)
  {
    std::string anOp = "(" + std::string(iOutputType) + ")";
    Expression& aResult = getGrammar().instantiateUnaryOperator(getAllocator(), iLeft, anOp);
    if (_observer)
    {
      return _observer->newUnary(aResult, iLeft, anOp);
    } else {
      return aResult;
    }
  }

  Expression& Parser::createFact(const char *iName)
  {
    std::string aTmpName(iName);
    Expression *aResult = NULL;

    if (_additionalFacts && _additionalFacts->hasFact(aTmpName))
    {
      std::string& aName = getAllocator().create<std::string>(iName);
      aResult = &_additionalFacts->instantiateFactResolver(aName);
    } else {
      aResult = &getGrammar().instantiateFactResolver(getAllocator(), aTmpName);
    }
    if (_observer)
    {
      return _observer->newFact(*aResult, aTmpName);
    } else {
      return *aResult;
    }
  }

  Expression& Parser::createAttribute(Expression& iInput, const char *iName)
  {
    Expression& aResult = getGrammar().instantiateAttributeResolver(getAllocator(), iInput, iName);
    if (_observer)
    {
      return _observer->newUnary(aResult, iInput, iName);
    } else {
      return aResult;
    }
  }

}}

