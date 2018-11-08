#pragma once
#include <string>
#include <map>
#include <vector>
#include <mdw/formula/Traits.hpp>
#include <mdw/formula/ArenaAllocator.hpp>
#include <boost/noncopyable.hpp>
#include <boost/foreach.hpp>

namespace mdw { namespace formula {

  class ArenaAllocator;
  class Grammar;
  class Expression;
  class Factorizer;

  class FactInstantiator {
  public:
    virtual ~FactInstantiator()
    {}

    virtual Expression& instantiate(ArenaAllocator& ioAllocator,
                                    const Grammar& iGrammar,
                                    const std::string& iName) const = 0;
  };

  class UnaryOpInstantiator {
  public:
    virtual ~UnaryOpInstantiator()
    {}

    virtual Expression& instantiate(ArenaAllocator& ioAllocator,
                                    const Grammar& ioGrammar,
                                    const std::string& iSymbol,
                                    const Expression& iChild) const = 0;
  };

  class BinaryOpInstantiator {
  public:
    virtual ~BinaryOpInstantiator()
    {}

    virtual Expression& instantiate(ArenaAllocator& ioAllocator,
                                    const Grammar& ioGrammar,
                                    const std::string& iSymbol,
                                    const Expression& iLeft,
                                    const Expression& iRight) const = 0;
  };

  class ArrowInstantiator {
  public:
    virtual ~ArrowInstantiator()
    {}

    // If there is a need to instantiate subFacts (temporary variables)
    virtual FactInstantiator *subFact(ArenaAllocator& ioAllocator,
                                      const Grammar& ioGrammar,
                                      const Expression& iLeft,
                                      const char *iName)
    {
      return NULL;
    }

    virtual Expression& instantiate(ArenaAllocator& ioAllocator,
                                    const Grammar& ioGrammar,
                                    const std::string& iSymbol,
                                    const Expression& iLeft,
                                    const Expression& iRight,
                                    const std::string& iName) const = 0;
  };

  /*
   * Grammar makes the link between type ids and type names.
   *
   */
  class Grammar: private boost::noncopyable {
    struct OperatorId {
      OperatorId(ExpressionType iLeftType, ExpressionType iRightType, const std::string& iSymbol);
      ExpressionType  _arg1Type;
      ExpressionType  _arg2Type;
      std::string     _symbol;

      bool operator<(const OperatorId& iOther) const;
    };

    struct Operator {
      Operator():
        _outputType(kExprVoid), _unary(kVoidType),
        _factInstantiator(NULL), _unaryInstantiator(NULL), _binaryInstantiator(NULL)
      {}

      Operator(ExpressionType iOutputType, UnaryOpInstantiator& ioInstantiator);
      Operator(ExpressionType iOutputType, BinaryOpInstantiator& ioInstantiator);
      Operator(ExpressionType iOutputType, ArrowInstantiator& ioInstantiator);
      Operator(ExpressionType iOutputType, FactInstantiator& ioInstantiator);

      ExpressionType  _outputType;
      enum IdentifierType {
        kVoidType,
        kUnary,
        kBinary,
        kObject,
        kAttribute
      } _unary;
      FactInstantiator *_factInstantiator;
      UnaryOpInstantiator *_unaryInstantiator;
      BinaryOpInstantiator *_binaryInstantiator;
      ArrowInstantiator *_arrowInstantiator;
    };

    std::map<std::string, ExpressionType> _types;
    std::map<OperatorId, Operator> _operators;
    std::vector<Factorizer*> _factorizers;
    ExpressionType _maxId;
    const Grammar *_chainedGrammar;

    ExpressionType findType(const char * iTypeName) const;

  public:
    Grammar();

    /// This is a chain of responsibility pattern.
    /// Make sure the linked grammar lives long enough, and don't do infinite loops!
    void linkGrammar(const Grammar& iChainedGrammar);

    void addObserver(Factorizer& ioFactorizer);

    template <class T> ExpressionType registerType();

    template <class T> ExpressionType findType() const;

    void registerStandardOperators(ArenaAllocator& ioAllocator);

    void registerUnaryOperator(ExpressionType iInputType,
                               ExpressionType iOutputType,
                               const std::string& iSymbol,
                               UnaryOpInstantiator& iInstantiator);
    
    void registerBinaryOperator(ExpressionType iLeftType,
                                ExpressionType iRightType,
                                ExpressionType iOutputType,
                                const std::string& iSymbol,
                                BinaryOpInstantiator& iInstantiator);
    
    void registerArrowOperator(ExpressionType iLeftType,
                                 ExpressionType iRightType,
                                 ExpressionType iOutputType,
                                 const std::string& iSymbol,
                                 ArrowInstantiator& iInstantiator);

    void registerFactResolver(const std::string& iName,
                              ExpressionType iOutputType,
                              FactInstantiator& iInstantiator);

    template <class T>
      void registerFactResolver(const std::string& iName,
                                FactInstantiator& iInstantiator)
      {
        ExpressionType aType = registerType<T>();
        registerFactResolver(iName, aType, iInstantiator);
      }

    bool hasFact(const std::string& iName) const;

    void registerAttributeResolver(ExpressionType iInputType,
                                   ExpressionType iOutputType,
                                   const std::string& iSymbol,
                                   UnaryOpInstantiator& iInstantiator);

    Expression& instantiateUnaryOperator(ArenaAllocator& ioAllocator,
                                         const Expression& iChild,
                                         const std::string& iSymbol) const;

    Expression& instantiateBinaryOperator(ArenaAllocator& ioAllocator,
                                          const Expression& iLeft,
                                          const Expression& iRight,
                                          const std::string& iSymbol) const;

    Expression& instantiateArrowOperator(ArenaAllocator& ioAllocator,
                                         const Expression& iLeft,
                                         const Expression& iRight,
                                         const char *iSymbol) const;

    Expression& instantiateFactResolver(ArenaAllocator& ioAllocator,
                                        const std::string& iObject) const;

    Expression& instantiateAttributeResolver(ArenaAllocator& ioAllocator,
                                             const Expression& iFact,
                                             const std::string& iAttribute) const;

    FactInstantiator *instantiateLocal(ArenaAllocator& ioAllocator,
                                       const Expression& iLeft,
                                       const char *iIdentifier) const;
  };

  template <> ExpressionType Grammar::findType<char>() const;
  template <> ExpressionType Grammar::findType<int8_t>() const;
  template <> ExpressionType Grammar::findType<uint8_t>() const;
  template <> ExpressionType Grammar::findType<int16_t>() const;
  template <> ExpressionType Grammar::findType<uint16_t>() const;
  template <> ExpressionType Grammar::findType<int32_t>() const;
  template <> ExpressionType Grammar::findType<uint32_t>() const;
  template <> ExpressionType Grammar::findType<int64_t>() const;
  template <> ExpressionType Grammar::findType<uint64_t>() const;

  template <> ExpressionType Grammar::findType<bool>() const;

  template <> ExpressionType Grammar::findType<double>() const;
  template <> ExpressionType Grammar::findType<float>() const;

  template <> ExpressionType Grammar::findType<std::string>() const;

}}

#include <mdw/formula/impl/Grammar.hpp>


