#pragma once
#include <string>
#include <mdw/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <mdw/formula/ArenaAllocator.hpp>
#include <mdw/formula/Traits.hpp>

namespace mdw { namespace formula {

  /*
   * Expression is the base class for the representation of any expression
   * that just returns a type so that the caller can dynamically cast it
   * to the right Expression type (StringExpression, DoubleExpression, IntExpression,
   * BoolExpression, DateExpression, TimeExpression).
   * These typed expression define a virtual evaluate() method to be called
   * by the operators.
   *
   * The expression on top of the tree constructed by parsing a formula has to be called
   * explicitely in order to destroy all the allocated objects.
   *
   */
  template <class T> class TypedExpression;
  class IContext;
  class Expression;
  class Grammar;

  class Expression: private boost::noncopyable {
  protected:
    ExpressionType _realType;

  public:
    Expression(ExpressionType iRealType):
      _realType(iRealType)
    {
    }

    template <class T> TypedExpression<T>& get()
    {
      try {
        return dynamic_cast<TypedExpression<T>& >(*this);
      } catch(...) {
        throw mdw::UnknownException("Expression " + toString() + " is not of type " +
                                    std::string(TypeTraits<T>::kTypeAsString) + " but " +
                                    mdw::lexical_cast<std::string>(_realType));
      }
    }

    template <class T> const TypedExpression<T>& get() const
    {
      try {
        return dynamic_cast<const TypedExpression<T>& >(*this);
      } catch(...) {
        throw mdw::UnknownException("Expression " + toString() + " is not of type " +
                                    std::string(TypeTraits<T>::kTypeAsString) + " but " +
                                    mdw::lexical_cast<std::string>(_realType));
      }
    }

    virtual TypedExpression<std::string>& getString();
    virtual TypedExpression<bool>& getBool();
    virtual TypedExpression<int>& getInt();
    virtual TypedExpression<int64_t>& getLong();
    virtual TypedExpression<double>& getDouble();

    virtual const TypedExpression<std::string>& getString() const;
    virtual const TypedExpression<bool>& getBool() const;
    virtual const TypedExpression<int>& getInt() const;
    virtual const TypedExpression<int64_t>& getLong() const;
    virtual const TypedExpression<double>& getDouble() const;

    ExpressionType getType() const {
      return _realType;
    }

    virtual std::string toString() const = 0;
    virtual std::string getTypeAsString() const = 0;
    virtual bool byValue() const = 0;

    // Unitary complexity (does not include subexpressions)
    // 1 corresponds to a basic operation (getter, integral operation...)
    virtual size_t complexity() const;

    virtual Expression& createChoice(ArenaAllocator& ioAllocator,
                                     const Grammar& iGrammar,
                                     const TypedExpression<bool>& iCondition,
                                     const Expression& iLeft,
                                     const Expression& iRight) const = 0;

  };

  template <class T> class BaseTypedExpression: public Expression
  {
    ExpressionType acquireType(const Grammar& iGrammar) const;

  public:
    BaseTypedExpression(const Grammar& iGrammar):
      Expression(acquireType(iGrammar))
    {}

    BaseTypedExpression(ExpressionType iType):
      Expression(iType)
    {}

    typedef T TechnicalType;
    typedef typename TypeTraits<T>::ReturnType ReturnType;

    std::string getTypeAsString() const {
      return TypeTraits<T>::kTypeAsString;
    }

    bool byValue() const {
      return __TypeTraits<typename TypeTraits<T>::ReturnType>::_IsBase;
    }

    virtual ReturnType evaluate(IContext& ioContext) const = 0;

    Expression& createChoice(ArenaAllocator& ioAllocator,
                             const Grammar& iGrammar,
                             const TypedExpression<bool>& iCondition,
                             const Expression& iLeft,
                             const Expression& iRight) const;

  };

  template <class T> class TypedExpression: public BaseTypedExpression<T>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      BaseTypedExpression<T>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      BaseTypedExpression<T>(iType)
    {}

  };

  template <> class TypedExpression<int>: public BaseTypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      BaseTypedExpression<int>(iGrammar)
    {
      if (getType() != kExprInt)
      {
        throw mdw::UnknownException("Expression type should be int, not " +
                                    mdw::lexical_cast<std::string>(getType()));
      }
    }

    TypedExpression(ExpressionType):
      BaseTypedExpression<int>(kExprInt)
    {}

    TypedExpression():
      BaseTypedExpression<int>(kExprInt)
    {}

    TypedExpression<int>& getInt(){
      return *this;
    }
    const TypedExpression<int>& getInt() const {
      return *this;
    }
  };

  template <> class TypedExpression<char>: public TypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<int>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<int>(iType)
    {}

    TypedExpression():
      TypedExpression<int>()
    {}
  };

  template <> class TypedExpression<int8_t>: public TypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<int>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<int>(iType)
    {}

    TypedExpression():
      TypedExpression<int>()
    {}
  };

  template <> class TypedExpression<uint8_t>: public TypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<int>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<int>(iType)
    {}

    TypedExpression():
      TypedExpression<int>()
    {}
  };

  template <> class TypedExpression<int16_t>: public TypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<int>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<int>(iType)
    {}

    TypedExpression():
      TypedExpression<int>()
    {}
  };

  template <> class TypedExpression<uint16_t>: public TypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<int>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<int>(iType)
    {}

    TypedExpression():
      TypedExpression<int>()
    {}
  };

  /*template <> class TypedExpression<int32_t>: public TypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<int>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<int>(iType)
    {}

    TypedExpression():
      TypedExpression<int>()
    {}
  };*/

  template <> class TypedExpression<uint32_t>: public TypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<int>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<int>(iType)
    {}

    TypedExpression():
      TypedExpression<int>()
    {}
  };

  template <> class TypedExpression<int64_t>: public TypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<int>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<int>(iType)
    {}

    TypedExpression():
      TypedExpression<int>()
    {}

    TypedExpression<int64_t>& getLong(){
      return *this;
    }
    const TypedExpression<int64_t>& getLong() const {
      return *this;
    }
  };

  template <> class TypedExpression<uint64_t>: public TypedExpression<int>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<int>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<int>(iType)
    {}

    TypedExpression():
      TypedExpression<int>()
    {}
  };

  template <> class TypedExpression<std::string>: public BaseTypedExpression<std::string>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      BaseTypedExpression<std::string>(iGrammar)
    {
      if (getType() != kExprString)
      {
        throw mdw::UnknownException("Expression type should be string, not " +
                                    mdw::lexical_cast<std::string>(getType()));
      }
    }

    TypedExpression(ExpressionType iType):
      BaseTypedExpression<std::string>(kExprString)
    {}

    TypedExpression():
      BaseTypedExpression<std::string>(kExprString)
    {}

    TypedExpression<std::string>& getString(){
      return *this;
    }
    const TypedExpression<std::string>& getString() const {
      return *this;
    }
  };

  template <> class TypedExpression<bool>: public BaseTypedExpression<bool>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      BaseTypedExpression<bool>(iGrammar)
    {
      if (getType() != kExprBool)
      {
        throw mdw::UnknownException("Expression type should be bool, not " +
                                    mdw::lexical_cast<std::string>(getType()));
      }
    }

    TypedExpression(ExpressionType iType):
      BaseTypedExpression<bool>(kExprBool)
    {}

    TypedExpression():
      BaseTypedExpression<bool>(kExprBool)
    {}

    TypedExpression<bool>& getBool(){
      return *this;
    }
    const TypedExpression<bool>& getBool() const {
      return *this;
    }
  };

  template <> class TypedExpression<double>: public BaseTypedExpression<double>
  {
  public:
    TypedExpression(const Grammar& iGrammar):
      BaseTypedExpression<double>(iGrammar)
    {
      if (getType() != kExprDouble)
      {
        throw mdw::UnknownException("Expression type should be double, not " +
                                    mdw::lexical_cast<std::string>(getType()));
      }
    }

    TypedExpression(ExpressionType iType):
      BaseTypedExpression<double>(kExprDouble)
    {}

    TypedExpression():
      BaseTypedExpression<double>(kExprDouble)
    {}

    TypedExpression<double>& getDouble(){
      return *this;
    }
    const TypedExpression<double>& getDouble() const {
      return *this;
    }
  };

  template <> class TypedExpression<float>: public TypedExpression<double>
  {
    TypedExpression(const Grammar& iGrammar):
      TypedExpression<double>(iGrammar)
    {}

    TypedExpression(ExpressionType iType):
      TypedExpression<double>(iType)
    {}

    TypedExpression():
      TypedExpression<double>()
    {}
  };

 template <class OutputType> class ChoiceOperator:
   public TypedExpression<OutputType>
  {
  public:
    typedef typename TypeTraits<OutputType>::ReturnType ReturnType;

    ChoiceOperator(const TypedExpression<bool>& iCondition,
                   const Grammar& iGrammar,
                   const TypedExpression<OutputType>& iFirst,
                   const TypedExpression<OutputType>& iSecond):
      TypedExpression<OutputType>(iGrammar),
      _condition(iCondition), _first(iFirst), _second(iSecond)
    {}

    ReturnType evaluate(IContext& ioContext) const
    {
      if (_condition.evaluate(ioContext))
      {
        return _first.evaluate(ioContext);
      } else {
        return _second.evaluate(ioContext);
      }
    }

    std::string toString() const {
      return "(" + _condition.toString() + ") ? ("
        + _first.toString() + ") : (" + _second.toString() + ")";
    }

  private:
    const TypedExpression<bool>& _condition;
    const TypedExpression<OutputType>& _first;
    const TypedExpression<OutputType>& _second;
  };

  template <class T>
    Expression& BaseTypedExpression<T>::createChoice(ArenaAllocator& ioAllocator,
                                                     const Grammar& iGrammar,
                                                     const TypedExpression<bool>& iCondition,
                                                     const Expression& iLeft,
                                                     const Expression& iRight) const
    {
      if (iLeft.getType() == iRight.getType() &&
          iLeft.getTypeAsString() == TypeTraits<T>::kTypeAsString)
      {
        const TypedExpression<T>& aLeft = iLeft.get<T>();
        const TypedExpression<T>& aRight = iRight.get<T>();

        return ioAllocator.create<ChoiceOperator<T> >(iCondition, iGrammar, aLeft, aRight);
      } else {
        throw mdw::UnknownException("Incompatible types in choice");
      }
    }

  template <> TypedExpression<bool>& Expression::get<bool>();
  template <> const TypedExpression<bool>& Expression::get<bool>() const;
  template <> TypedExpression<int64_t>& Expression::get<int64_t>();
  template <> const TypedExpression<int64_t>& Expression::get<int64_t>() const;
  template <> TypedExpression<int>& Expression::get<int>();
  template <> const TypedExpression<int>& Expression::get<int>() const;
  template <> TypedExpression<std::string>& Expression::get<std::string>();
  template <> const TypedExpression<std::string>& Expression::get<std::string>() const;
  template <> TypedExpression<double>& Expression::get<double>();
  template <> const TypedExpression<double>& Expression::get<double>() const;
}}

#include <mdw/formula/impl/Expression.hpp>

