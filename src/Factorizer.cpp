#include <mdw/formula/cache/Factorizer.hpp>
#include <mdw/UnknownException.hpp>
#include <boost/foreach.hpp>

namespace mdw { namespace formula {

  Factorizer::KnownExpression::KnownExpression(Expression& ioExpression, Factorizer& ioParent):
    _expression(ioExpression), _parent(ioParent), _type(ioExpression.getType()),
    _totalComplexity(ioExpression.complexity()), _toString(ioExpression.toString()),
    _optimized(&ioExpression)
  {}

  void Factorizer::KnownExpression::addDependency(const Expression& iExpression)
  {
    KnownExpression *aKnown = _parent.getKnown(iExpression);
    if (aKnown)
    {
      _usedFacts.insert(aKnown->_usedFacts.begin(), aKnown->_usedFacts.end());
      _totalComplexity += aKnown->_totalComplexity;
    } else {
      throw mdw::UnknownException("Could not find known expression: " + iExpression.toString());
    }
  }

  Factorizer::Factorizer():
    Observer(_allocator)
  {}

  Factorizer::~Factorizer()
  {
    reset();

    Types::iterator anIt = _types.begin();
    Types::iterator anEnd = _types.end();
    while (anIt != anEnd)
    {
      delete anIt->second;
      ++anIt;
    }
    _types.clear();
  }

  void Factorizer::reset()
  {
    _expressions.clear();

    _displays.clear();

    _facts.clear();
    getAllocator().clean();
  }

  Factorizer::KnownExpression *Factorizer::getByDisplay(const std::string& iDisplay)
  {
    ByDisplay::iterator anExp = _displays.find(iDisplay);
    if (anExp != _displays.end())
    {
      KnownExpressions::iterator aKnown = _expressions.find(anExp->second);
      if (aKnown != _expressions.end())
      {
        return aKnown->second;
      }
    }
    return NULL;
  }

  Factorizer::KnownExpression *Factorizer::getKnown(const Expression& iExpression)
  {
    KnownExpressions::iterator aKnown = _expressions.find(&iExpression);
    if (aKnown != _expressions.end())
    {
      return aKnown->second;
    } else {
      return NULL;
    }
  }

  Factorizer::KnownExpression& Factorizer::createKnown(Expression& ioExpression,
                                                       const std::string& iDisplay)
  {
    KnownExpression *anExpr = &getAllocator().create<KnownExpression>(ioExpression, *this);
    _expressions[&ioExpression] = anExpr;
    _displays[iDisplay] = &ioExpression;
    return *anExpr;
  }

  Expression& Factorizer::optimize(KnownExpression& ioKnown)
  {
    if (ioKnown._usedFacts.empty())
    {
      Types::iterator aTypeIt = _types.find(ioKnown._type);
      if ((aTypeIt != _types.end()) && (aTypeIt->second != NULL))
      {
        Expression& aConstant =
          aTypeIt->second->getConstant(ioKnown._expression, ioKnown._type, getAllocator());
        ioKnown._optimized = &aConstant;
        _expressions[&aConstant] = &ioKnown;
        FORMULA_DEBUG("Optimized constant expression: " << aConstant.toString());
      } else {
        FORMULA_DEBUG("Missing type for constant: " << ioKnown._expression.toString());
      }
    } else if ((ioKnown._totalComplexity > 5) && (ioKnown._usedFacts.size() == 1)) {
      Types::iterator aTypeIt = _types.find(ioKnown._type);
      if ((aTypeIt != _types.end()) && (aTypeIt->second != NULL))
      {
        Facts::iterator aFactIt = _facts.find(*ioKnown._usedFacts.begin());
        if ((aFactIt == _facts.end()) || (aFactIt->second == NULL))
        {
          FORMULA_DEBUG("Missing fact: " << *ioKnown._usedFacts.begin());
        } else {
          Expression& aCached =
            aTypeIt->second->getUnaryCached(ioKnown._expression, *aFactIt->second, getAllocator());
          ioKnown._optimized = &aCached;
          ioKnown._totalComplexity = aCached.complexity();
          _expressions[&aCached] = &ioKnown;
          FORMULA_DEBUG("Optimized unary expression: " << aCached.toString());
        }
      } else {
        FORMULA_DEBUG("Missing type for unary: " << ioKnown._expression.toString());
      }
    }
    return *ioKnown._optimized;
  }

  Expression& Factorizer::newConstant(Expression& ioResult)
  {
    std::string aDisplay = ioResult.toString();
    KnownExpression *aKnown = getByDisplay(aDisplay);
    if (aKnown)
    {
      return *aKnown->_optimized;
    } else {
      createKnown(ioResult, aDisplay);
      return ioResult;
    }
  }

  Expression& Factorizer::newFact(Expression& ioResult, const std::string& iName)
  {
    std::string aDisplay = ioResult.toString();
    KnownExpression *aKnown = getByDisplay(aDisplay);
    if (aKnown)
    {
      return *aKnown->_optimized;
    } else {
      KnownExpression& anExpr = createKnown(ioResult, aDisplay);
      anExpr._usedFacts.insert(iName);

      Types::iterator aFactTypeIt = _types.find(ioResult.getType());
      if ((aFactTypeIt != _types.end()) && (aFactTypeIt->second != NULL))
      {
        FactByAddress& aFactPtr = aFactTypeIt->second->getFactByAddress(ioResult, getAllocator());
        _facts[iName] = &aFactPtr;
      } else {
        FORMULA_DEBUG("Missing fact type for " << iName);
      }
      return ioResult;
    }
  }

  Expression& Factorizer::newUnary(Expression& ioResult,
                                   Expression& ioRight,
                                   const std::string& iSymbol)
  {
    std::string aDisplay = ioResult.toString();
    KnownExpression *aKnown = getByDisplay(aDisplay);
    if (aKnown)
    {
      return *aKnown->_optimized;
    } else {
      KnownExpression& anExpr = createKnown(ioResult, aDisplay);
      anExpr.addDependency(ioRight);
      return optimize(anExpr);
    }
  }

  Expression& Factorizer::newBinary(Expression& ioResult,
                                    Expression& ioLeft,
                                    Expression& ioRight,
                                    const std::string& iSymbol)
  {
    std::string aDisplay = ioResult.toString();
    KnownExpression *aKnown = getByDisplay(aDisplay);
    if (aKnown)
    {
      return *aKnown->_optimized;
    } else {
      KnownExpression& anExpr = createKnown(ioResult, aDisplay);
      anExpr.addDependency(ioLeft);
      anExpr.addDependency(ioRight);
      return optimize(anExpr);
    }
  }

  Expression& Factorizer::newChoice(Expression& ioResult,
                                    TypedExpression<bool>& ioCondition,
                                    Expression& ioLeft,
                                    Expression& ioRight)
  {
    std::string aDisplay = ioResult.toString();
    KnownExpression *aKnown = getByDisplay(aDisplay);
    if (aKnown)
    {
      return *aKnown->_optimized;
    } else {
      KnownExpression& anExpr = createKnown(ioResult, aDisplay);
      anExpr.addDependency(ioCondition);
      anExpr.addDependency(ioLeft);
      anExpr.addDependency(ioRight);
      return optimize(anExpr);
    }
  }

  Expression& Factorizer::newArrow(Expression& ioResult,
                                   Expression& ioContainer,
                                   Expression& ioCondition,
                                   const std::string& iLocalName)

  {
    std::string aDisplay = ioResult.toString();
    KnownExpression *aKnown = getByDisplay(aDisplay);
    if (aKnown)
    {
      return *aKnown->_optimized;
    } else {
      KnownExpression& anExpr = createKnown(ioResult, aDisplay);
      anExpr.addDependency(ioContainer);
      anExpr.addDependency(ioCondition);
      anExpr._usedFacts.erase(iLocalName);
      return optimize(anExpr);
    }
  }

}}

