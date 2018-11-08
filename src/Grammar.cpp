#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Operator.hpp>
#include <mdw/formula/Parser.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/StandardTypes.hpp>
#include <mdw/formula/cache/Factorizer.hpp>
#include <functional>
#include <string>

namespace mdw { namespace formula {

  Grammar::Grammar():
    _maxId(kExprMaxType), _chainedGrammar(NULL)
  {
    _types["int"] = kExprInt;
    _types["bool"] = kExprBool;
    _types["double"] = kExprDouble;
    _types["string"] = kExprString;
  }

  void Grammar::linkGrammar(const Grammar& iChainedGrammar)
  {
    _chainedGrammar = &iChainedGrammar;
  }

  void Grammar::addObserver(Factorizer& ioFactorizer)
  {
    ioFactorizer.registerType<bool>(*this);
    ioFactorizer.registerType<int>(*this);
    ioFactorizer.registerType<double>(*this);
    ioFactorizer.registerType<std::string>(*this);

    _factorizers.push_back(&ioFactorizer);
  }

  Grammar::OperatorId::OperatorId(ExpressionType iLeftType,
                                  ExpressionType iRightType,
                                  const std::string& iSymbol):
    _arg1Type(iLeftType), _arg2Type(iRightType), _symbol(iSymbol)
  {
  }

  bool Grammar::OperatorId::operator<(const OperatorId& iOther) const
  {
    return
      (_arg1Type < iOther._arg1Type) ||
      ((_arg1Type == iOther._arg1Type) &&
       ((_arg2Type < iOther._arg2Type) ||
        ((_arg2Type == iOther._arg2Type) && (_symbol < iOther._symbol))));
  }

  Grammar::Operator::Operator(ExpressionType iOutputType,
                              UnaryOpInstantiator& ioInstantiator):
    _outputType(iOutputType), _unary(kUnary),
    _factInstantiator(NULL), _unaryInstantiator(&ioInstantiator), _binaryInstantiator(NULL),
    _arrowInstantiator(NULL)
  {}

  Grammar::Operator::Operator(ExpressionType iOutputType,
                              BinaryOpInstantiator& ioInstantiator):
    _outputType(iOutputType), _unary(kBinary), _factInstantiator(NULL), _unaryInstantiator(NULL),
    _binaryInstantiator(&ioInstantiator), _arrowInstantiator(NULL)
  {}

  Grammar::Operator::Operator(ExpressionType iOutputType,
                              ArrowInstantiator& ioInstantiator):
    _outputType(iOutputType), _unary(kBinary), _factInstantiator(NULL), _unaryInstantiator(NULL),
    _binaryInstantiator(NULL), _arrowInstantiator(&ioInstantiator)
  {}

  Grammar::Operator::Operator(ExpressionType iOutputType,
                              FactInstantiator& ioInstantiator):
    _outputType(iOutputType), _unary(kObject), _factInstantiator(&ioInstantiator),
    _unaryInstantiator(NULL), _binaryInstantiator(NULL), _arrowInstantiator(NULL)
  {}

  ExpressionType Grammar::findType(const char * iTypeName) const
  {
    std::map<std::string, ExpressionType>::const_iterator anIt = _types.find(iTypeName);
    if (anIt == _types.end())
    {
      return kExprVoid;
    } else if (_chainedGrammar) {
      return _chainedGrammar->findType(iTypeName);
    } else {
      return anIt->second;
    }
  }

  void Grammar::registerStandardOperators(ArenaAllocator& ioAllocator)
  {
    StandardUnary::RegisterMe(*this, ioAllocator);
    StandardBinary::RegisterMe(*this, ioAllocator);
  }

  void Grammar::registerUnaryOperator(ExpressionType iInputType,
                                      ExpressionType iOutputType,
                                      const std::string& iSymbol,
                                      UnaryOpInstantiator& iInstantiator)
  {
    OperatorId anId(iInputType, kExprVoid, iSymbol);
    Operator anOperator(iOutputType, iInstantiator);

    _operators[anId] = anOperator;
  }

  void Grammar::registerBinaryOperator(ExpressionType iLeftType,
                                       ExpressionType iRightType,
                                       ExpressionType iOutputType,
                                       const std::string& iSymbol,
                                       BinaryOpInstantiator& iInstantiator)
  {
    OperatorId anId(iLeftType, iRightType, iSymbol);
    Operator anOperator(iOutputType, iInstantiator);

    _operators[anId] = anOperator;
  }

  void Grammar::registerArrowOperator(ExpressionType iLeftType,
                                        ExpressionType iRightType,
                                        ExpressionType iOutputType,
                                        const std::string& iSymbol,
                                        ArrowInstantiator& iInstantiator)
  {
    OperatorId anId(iLeftType, iRightType, iSymbol);
    Operator anOperator(iOutputType, iInstantiator);

    _operators[anId] = anOperator;

    OperatorId aFactId(iLeftType, kExprVoid, iSymbol);
    _operators[aFactId] = anOperator;
  }

  void Grammar::registerFactResolver(const std::string& iName,
                                     ExpressionType iOutputType,
                                     FactInstantiator& iInstantiator)
  {
    OperatorId anId(kExprVoid, kExprVoid, iName);
    Operator anOperator(iOutputType, iInstantiator);

    _operators[anId] = anOperator;
  }

  void Grammar::registerAttributeResolver(ExpressionType iInputType,
                                          ExpressionType iOutputType,
                                          const std::string& iSymbol,
                                          UnaryOpInstantiator& iInstantiator)
  {
    registerUnaryOperator(iInputType, iOutputType, iSymbol, iInstantiator);
  }

  Expression& Grammar::instantiateUnaryOperator(ArenaAllocator& ioAllocator,
                                                const Expression& iChild,
                                                const std::string& iSymbol) const
  {
    std::map<OperatorId, Operator>::const_iterator anIt =
      _operators.find(OperatorId(iChild.getType(), kExprVoid, iSymbol));
    if (anIt != _operators.end()
        && anIt->second._unary == Operator::kUnary
        && anIt->second._unaryInstantiator != NULL)
    {
      return anIt->second._unaryInstantiator->instantiate(ioAllocator, *this,
                                                          anIt->first._symbol, iChild);
    } else if (_chainedGrammar) {
      return _chainedGrammar->instantiateUnaryOperator(ioAllocator, iChild, iSymbol);
    } else {
      throw mdw::UnknownException("Unary operator not found: " + iSymbol + " on type " +
                                  std::string(iChild.getTypeAsString()));
    }
  }

  Expression& Grammar::instantiateBinaryOperator(ArenaAllocator& ioAllocator,
                                                 const Expression& iLeft,
                                                 const Expression& iRight,
                                                 const std::string& iSymbol) const
  {
    std::map<OperatorId, Operator>::const_iterator anIt =
      _operators.find(OperatorId(iLeft.getType(), iRight.getType(), iSymbol));
    if (anIt != _operators.end() 
        && anIt->second._unary == Operator::kBinary
        && anIt->second._binaryInstantiator != NULL)
    {
      return anIt->second._binaryInstantiator->instantiate(ioAllocator, *this, anIt->first._symbol,
                                                           iLeft, iRight);
    } else if (_chainedGrammar) {
      return _chainedGrammar->instantiateBinaryOperator(ioAllocator, iLeft, iRight, iSymbol);
    } else {
      throw mdw::UnknownException("Binary operator not found: " + iSymbol + " between types " +
                                  std::string(iLeft.getTypeAsString()) + " (" +
                                  mdw::lexical_cast<std::string>(iLeft.getType()) +
                                  ") and " + std::string(iRight.getTypeAsString()) + " (" +
                                  mdw::lexical_cast<std::string>(iRight.getType()) + ")");
    }
  }

  Expression& Grammar::instantiateArrowOperator(ArenaAllocator& ioAllocator,
                                                const Expression& iLeft,
                                                const Expression& iRight,
                                                const char *iSymbol) const
  {
    std::map<OperatorId, Operator>::const_iterator anIt =
      _operators.find(OperatorId(iLeft.getType(), iRight.getType(), "->"));
    if (anIt != _operators.end() 
        && anIt->second._unary == Operator::kBinary
        && anIt->second._arrowInstantiator != NULL)
    {
      const std::string& aSymbol = ioAllocator.create<std::string>(iSymbol);
      return anIt->second._arrowInstantiator->instantiate(ioAllocator, *this, anIt->first._symbol,
                                                          iLeft, iRight, aSymbol);
    } else if (_chainedGrammar) {
      return _chainedGrammar->instantiateArrowOperator(ioAllocator, iLeft, iRight, iSymbol);
    } else {
      throw mdw::UnknownException("Binary operator not found: -> between types " +
                                  std::string(iLeft.getTypeAsString()) + " (" +
                                  mdw::lexical_cast<std::string>(iLeft.getType()) +
                                  ") and " + std::string(iRight.getTypeAsString()) + " (" +
                                  mdw::lexical_cast<std::string>(iRight.getType()) + ")");
    }
  }

  FactInstantiator *Grammar::instantiateLocal(ArenaAllocator& ioAllocator,
                                              const Expression& iLeft,
                                              const char *iIdentifier) const
  {
    std::map<OperatorId, Operator>::const_iterator anIt =
      _operators.find(OperatorId(iLeft.getType(), kExprVoid, "->"));
    if (anIt != _operators.end() 
        && anIt->second._unary == Operator::kBinary
        && anIt->second._arrowInstantiator != NULL)
    {
      return anIt->second._arrowInstantiator->subFact(ioAllocator, *this, iLeft, iIdentifier);
    } else if (_chainedGrammar) {
      return _chainedGrammar->instantiateLocal(ioAllocator, iLeft, iIdentifier);
    } else {
      throw mdw::UnknownException("Fact instantiator not found: -> on type " +
                                  std::string(iLeft.getTypeAsString()) + " (" +
                                  mdw::lexical_cast<std::string>(iLeft.getType()) +
                                  ")");
    }
  }

  bool Grammar::hasFact(const std::string& iName) const
  {
    std::map<OperatorId, Operator>::const_iterator anIt =
      _operators.find(OperatorId(kExprVoid, kExprVoid, iName));
    if (anIt != _operators.end() 
        && anIt->second._unary == Operator::kObject 
        && anIt->second._factInstantiator != NULL)
    {
      return true;
    } else if (_chainedGrammar) {
      return _chainedGrammar->hasFact(iName);
    } else {
      return false;
    }
  }

  Expression& Grammar::instantiateFactResolver(ArenaAllocator& ioAllocator,
                                               const std::string& iObject) const
  {
    std::map<OperatorId, Operator>::const_iterator anIt =
      _operators.find(OperatorId(kExprVoid, kExprVoid, iObject));
    if (anIt != _operators.end() 
        && anIt->second._unary == Operator::kObject 
        && anIt->second._factInstantiator != NULL)
    {
      return anIt->second._factInstantiator->instantiate(ioAllocator, *this, anIt->first._symbol);
    } else if (_chainedGrammar) {
      return _chainedGrammar->instantiateFactResolver(ioAllocator, iObject);
    } else {
      throw mdw::UnknownException("Fact resolver not found: " + iObject);
    }
  }

  Expression& Grammar::instantiateAttributeResolver(ArenaAllocator& ioAllocator,
                                                    const Expression& iFact,
                                                    const std::string& iAttribute) const
  {
    return instantiateUnaryOperator(ioAllocator, iFact, iAttribute);
  }

  template <> ExpressionType Grammar::findType<char>() const
  {
    return kExprInt;
  }
  template <> ExpressionType Grammar::findType<int8_t>() const
  {
    return kExprInt;
  }
  template <> ExpressionType Grammar::findType<uint8_t>() const
  {
    return kExprInt;
  }
  template <> ExpressionType Grammar::findType<int16_t>() const
  {
    return kExprInt;
  }
  template <> ExpressionType Grammar::findType<uint16_t>() const
  {
    return kExprInt;
  }
  template <> ExpressionType Grammar::findType<int32_t>() const
  {
    return kExprInt;
  }
  template <> ExpressionType Grammar::findType<uint32_t>() const
  {
    return kExprInt;
  }
  template <> ExpressionType Grammar::findType<int64_t>() const
  {
    return kExprInt;
  }
  template <> ExpressionType Grammar::findType<uint64_t>() const
  {
    return kExprInt;
  }

  template <> ExpressionType Grammar::findType<bool>() const
  {
    return kExprBool;
  }

  template <> ExpressionType Grammar::findType<double>() const
  {
    return kExprDouble;
  }
  template <> ExpressionType Grammar::findType<float>() const
  {
    return kExprDouble;
  }

  template <> ExpressionType Grammar::findType<std::string>() const
  {
    return kExprString;
  }

}}
