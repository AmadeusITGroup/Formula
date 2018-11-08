#pragma once
#include <string>
#include <map>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/IContext.hpp>

namespace mdw { namespace formula {

  template <> struct TypeTraits <const char *, void>
  {
    typedef const char *ReturnType;
    static const char kTypeAsString[];
  };

  template <> class __TypeTraits <const char *, void>
  {
  public:
    typedef const char *actual_type;
    static const bool _IsBase = true;
    static const void *ToVoid(const char *iValue) {
      return iValue;
    }
    typedef const char *cached_type;
    typedef std::string key_type;
    static cached_type ToCached(const char *iValue) {
      return iValue;
    }
    static const char *FromCached(cached_type iValue) {
      return iValue;
    }
  };

  class CString
  {
    class CStringUnary: public UnaryOpInstantiator
    {
    public:
      Expression& instantiate(ArenaAllocator& ioAllocator,
                              const Grammar& iGrammar,
                              const std::string& iSymbol,
                              const Expression& iChild) const;

      static void RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator);
    };

    class CStringBinary: public BinaryOpInstantiator
    {
    public:
      Expression& instantiate(ArenaAllocator& ioAllocator,
                              const Grammar& iGrammar,
                              const std::string& iSymbol,
                              const Expression& iLeft,
                              const Expression& iRight) const;

      static void RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator);
    };

  public:
    Expression& instantiate(ArenaAllocator& ioAllocator,
                            const Grammar& iGrammar,
                            const std::string& iSymbol,
                            const Expression& iChild) const;

    Expression& instantiate(ArenaAllocator& ioAllocator,
                            const Grammar& iGrammar,
                            const std::string& iSymbol,
                            const Expression& iLeft,
                            const Expression& iRight) const;

    static void RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator);
  };

  class StringToCStringComparator: public TypedExpression<bool>
  {
    const TypedExpression<std::string>& _left;
    const TypedExpression<const char*>& _right;
    const std::string& _symbol;
    bool _isEqualOK;
    bool _isGreaterOK;
    bool _isLowerOK;
    bool _revert;

  public:
    StringToCStringComparator(const TypedExpression<std::string>& iLeft,
                              const TypedExpression<const char *>& iRight,
                              const std::string& iSymbol,
                              bool iRevert);

    std::string toString() const;

    const TypedExpression<std::string>& getLeft() const
    {
      return _left;
    }

    const TypedExpression<const char*>& getRight() const
    {
      return _right;
    }

    bool evaluate(IContext& ioContext) const;
  };

  class CStringComparator: public TypedExpression<bool>
  {
    const TypedExpression<const char*>& _left;
    const TypedExpression<const char*>& _right;
    const std::string& _symbol;
    bool _isEqualOK;
    bool _isGreaterOK;
    bool _isLowerOK;

  public:
    CStringComparator(const TypedExpression<const char *>& iLeft,
                      const TypedExpression<const char *>& iRight,
                      const std::string& iSymbol);

    std::string toString() const;

    const TypedExpression<const char*>& getLeft() const
    {
      return _left;
    }

    const TypedExpression<const char*>& getRight() const
    {
      return _right;
    }

    bool evaluate(IContext& ioContext) const;
  };
}}

