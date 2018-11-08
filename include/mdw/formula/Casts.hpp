#pragma once
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/IContext.hpp>
#include <mdw/lexical_cast.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/utility/enable_if.hpp>

/*
 *  Operators define the common operations on Expressions.
 *  All operands and operators must live longer than their parent expressions.
 *
 */

namespace mdw { namespace formula {

  template <class InputT, class OutputT, class isObject = void> class ExpressionCast:
    public TypedExpression<OutputT>
  {
    const TypedExpression<InputT>& _expression;
    const std::string& _symbol;
  public:
    typedef typename TypeTraits<OutputT>::ReturnType ReturnType;

    ExpressionCast(const TypedExpression<InputT>& iExpression, const Grammar& iGrammar,
                   const std::string& iSymbol):
      TypedExpression<OutputT>(iGrammar), _expression(iExpression), _symbol(iSymbol)
    {}

    ReturnType evaluate(IContext& ioContext) const
    {
      // Return an instance on the stack for copied objects
      return static_cast<ReturnType>(_expression.evaluate(ioContext));
    }

    std::string toString() const
    {
      return _symbol + "(" + _expression.toString() + ")";
    }
  };

  template <class InputT, class OutputT>
    class ExpressionCast<InputT, OutputT, typename boost::enable_if<boost::is_reference<typename TypeTraits<OutputT>::ReturnType> >::type>:
    public TypedExpression<OutputT>
  {
    const TypedExpression<InputT>& _expression;
    const std::string& _symbol;
  public:
    typedef typename TypeTraits<OutputT>::ReturnType ReturnType;

    ExpressionCast(const TypedExpression<InputT>& iExpression, const Grammar& iGrammar,
                   const std::string& iSymbol):
      TypedExpression<OutputT>(iGrammar), _expression(iExpression), _symbol(iSymbol)
    {}

    ReturnType evaluate(IContext& ioContext) const
    {
      // Allocate an instance for objects
      return ioContext.getAllocator().template create<OutputT>(_expression.evaluate(ioContext));
    }

    std::string toString() const
    {
      return _symbol + "(" + _expression.toString() + ")";
    }
  };

  // Avoid stupid C++ conversions where 0.99999995 is cast to 0 as an integer
  template <> class ExpressionCast<double, int, void>:
    public TypedExpression<int>
    {
      const TypedExpression<double>& _expression;
    public:
      typedef TypeTraits<int>::ReturnType ReturnType;

      ExpressionCast(const TypedExpression<double>& iExpression,
                     const Grammar& iGrammar,
                   const std::string& iSymbol):
        _expression(iExpression)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        double aValue = _expression.evaluate(ioContext);
        return (ReturnType) (aValue >= 0 ? aValue + 0.5 : aValue - 0.5);
      }

      std::string toString() const
      {
        return "(int)(" + _expression.toString() + ")";
      }
    };

  template <class T> class ExpressionCast<T, std::string, void>:
    public TypedExpression<std::string>
    {
      const TypedExpression<T>& _expression;
    public:
      typedef TypeTraits<std::string>::ReturnType ReturnType;

      ExpressionCast(const TypedExpression<T>& iExpression,
                     const Grammar& iGrammar,
                   const std::string& iSymbol):
        _expression(iExpression)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        std::string aValue = mdw::lexical_cast<std::string>(_expression.evaluate(ioContext));
        return ioContext.getAllocator().template create<std::string>(aValue);
      }

      std::string toString() const
      {
        return "(string)(" + _expression.toString() + ")";
      }
    };

  template <> class ExpressionCast<std::string, int, void>:
    public TypedExpression<int>
    {
      const TypedExpression<std::string>& _expression;
    public:
      typedef TypeTraits<int>::ReturnType ReturnType;

      ExpressionCast(const TypedExpression<std::string>& iExpression,
                     const Grammar& iGrammar,
                   const std::string& iSymbol):
        _expression(iExpression)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        return atoi(_expression.evaluate(ioContext).c_str());
      }

      std::string toString() const
      {
        return "(int)(" + _expression.toString() + ")";
      }
    };

  template <> class ExpressionCast<std::string, double, void>:
    public TypedExpression<double>
    {
      const TypedExpression<std::string>& _expression;
    public:
      typedef TypeTraits<double>::ReturnType ReturnType;

      ExpressionCast(const TypedExpression<std::string>& iExpression,
                     const Grammar& iGrammar,
                   const std::string& iSymbol):
        _expression(iExpression)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        return atof(_expression.evaluate(ioContext).c_str());
      }

      std::string toString() const
      {
        return "(double)(" + _expression.toString() + ")";
      }
    };

  template <class From, class To>
    class DefaultCast: public UnaryOpInstantiator
  {
  public:
    Expression& instantiate(ArenaAllocator& ioAllocator,
                            const Grammar& iGrammar,
                            const std::string& iSymbol,
                            const Expression& iChild) const
    {
      return ioAllocator.create<ExpressionCast<From, To> >(iChild.get<From>(), iGrammar, iSymbol);
    }

    static void RegisterMe(ArenaAllocator& ioAllocator, Grammar& ioGrammar, const std::string& iSymbol)
    {
      ioGrammar.registerUnaryOperator(ioGrammar.registerType<From>(),
                                      ioGrammar.registerType<To>(),
                                      iSymbol,
                                      ioAllocator.create<DefaultCast<From, To> >());
    }
  };

}}


