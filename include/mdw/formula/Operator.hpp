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

  template <class InputT, class OutputT,
           class OpT = std::unary_function<typename TypedExpression<InputT>::ReturnType, typename TypeTraits<OutputT>::ReturnType> >
             class UnaryOperator:
               public TypedExpression<OutputT>
  {
  public:
    typedef typename TypeTraits<OutputT>::ReturnType ReturnType;
    typedef typename TypedExpression<InputT>::ReturnType InputType;

    UnaryOperator(const TypedExpression<InputT>& iRight,
                  const OpT& iOperator,
                  const Grammar& iGrammar,
                  const std::string& iSymbol) :
      TypedExpression<OutputT>(iGrammar), _right(iRight), _operator(iOperator), _symbol(iSymbol)
    {}


    ReturnType evaluate(IContext& ioContext) const {
      return _operator(_right.evaluate(ioContext));
    }

    std::string toString() const
    {
      return _symbol + "(" + _right.toString() + ")";
    } 

    const std::string& getSymbol() const
    {
      return _symbol;
    } 

  private:
    const TypedExpression<InputT>& _right;
    const OpT& _operator;
    const std::string& _symbol;

  };

  // Operator must be something such as std::unary_function<InputType, ReturnType>
  // And default-constructible.
  template <class InputT, class OutputT, class OperatorT> class TypedUnaryOperator:
    public TypedExpression<OutputT>
  {
  public:
    typedef typename TypeTraits<OutputT>::ReturnType ReturnType;
    typedef typename TypedExpression<InputT>::ReturnType InputType;

    TypedUnaryOperator(const TypedExpression<InputT>& iRight,
                       const Grammar& iGrammar,
                       const std::string& iSymbol) :
      TypedExpression<OutputT>(iGrammar), _right(iRight), _symbol(iSymbol)
    {}


    ReturnType evaluate(IContext& ioContext) const {
      return _operator(_right.evaluate(ioContext));
    }

    std::string toString() const
    {
      return _symbol + "(" + _right.toString() + ")";
    } 

    const std::string& getSymbol() const
    {
      return _symbol;
    } 

  private:
    const TypedExpression<InputT>& _right;
    OperatorT _operator;
    const std::string& _symbol;

  };

  template <class InputT, class OutputT,
           class OpT = std::binary_function<typename TypedExpression<InputT>::ReturnType, typename TypedExpression<InputT>::ReturnType, typename TypeTraits<OutputT>::ReturnType> >
             class SymmetricOperator:
               public TypedExpression<OutputT>
  {
  public:
    typedef typename TypeTraits<OutputT>::ReturnType ReturnType;
    typedef typename TypedExpression<InputT>::ReturnType InputType;

    SymmetricOperator(const TypedExpression<InputT>& iLeft,
                      const TypedExpression<InputT>& iRight,
                      const OpT& iOperator,
                      const Grammar& iGrammar,
                      const std::string& iSymbol) :
      TypedExpression<OutputT>(iGrammar), _left(iLeft), _right(iRight),
      _operator(iOperator), _symbol(iSymbol)
    {}


    ReturnType evaluate(IContext& ioContext) const {
      return _operator(_left.evaluate(ioContext), _right.evaluate(ioContext));
    }

    std::string toString() const
    {
      return "(" + _left.toString() + ")" + _symbol + "(" + _right.toString() + ")";
    } 

    const std::string& getSymbol() const
    {
      return _symbol;
    } 

  private:
    const TypedExpression<InputT>& _left;
    const TypedExpression<InputT>& _right;
    const OpT& _operator;
    const std::string& _symbol;

  };

  // Operator must be something such as binary_function<InputType, InputType, ReturnType>
  // And default-constructible.
  template <class InputT, class OutputT, class OperatorT> class SymmetricTypedOperator:
    public TypedExpression<OutputT>
  {
  public:
    typedef typename TypeTraits<OutputT>::ReturnType ReturnType;
    typedef typename TypedExpression<InputT>::ReturnType InputType;

    SymmetricTypedOperator(const TypedExpression<InputT>& iLeft,
                           const TypedExpression<InputT>& iRight,
                           const Grammar& iGrammar,
                           const std::string& iSymbol) :
      TypedExpression<OutputT>(iGrammar), _left(iLeft), _right(iRight), _symbol(iSymbol)
    {}

    virtual ~SymmetricTypedOperator() {};

    virtual ReturnType evaluate(IContext& ioContext) const {
      return _operator(_left.evaluate(ioContext), _right.evaluate(ioContext));
    }

    std::string toString() const
    {
      return "(" + _left.toString() + ")" + _symbol + "(" + _right.toString() + ")";
    } 

    const std::string& getSymbol() const
    {
      return _symbol;
    } 

    const TypedExpression<InputT>& getLeft() const
    {
      return _left;
    }

    const TypedExpression<InputT>& getRight() const
    {
      return _right;
    }
  protected:
    const TypedExpression<InputT>& _left;
    const TypedExpression<InputT>& _right;
    OperatorT _operator;
    const std::string& _symbol;

  };

  template <class InputT, class Input2T, class OutputT,
           class OpT = std::binary_function<typename TypedExpression<InputT>::ReturnType, typename TypedExpression<Input2T>::ReturnType, typename TypeTraits<OutputT>::ReturnType> >
             class AsymmetricOperator:
               public TypedExpression<OutputT>
  {
  public:
    typedef typename TypeTraits<OutputT>::ReturnType ReturnType;
    typedef typename TypedExpression<InputT>::ReturnType InputType;
    typedef typename TypedExpression<Input2T>::ReturnType Input2Type;

    AsymmetricOperator(const TypedExpression<InputT>& iLeft,
                       const TypedExpression<Input2T>& iRight,
                       const OpT& iOperator,
                       const Grammar& iGrammar,
                       const std::string& iSymbol) :
      TypedExpression<OutputT>(iGrammar), _left(iLeft), _right(iRight),
      _operator(iOperator), _symbol(iSymbol)
    {}


    ReturnType evaluate(IContext& ioContext) const {
      return _operator(_left.evaluate(ioContext), _right.evaluate(ioContext));
    }

    std::string toString() const
    {
      return "(" + _left.toString() + ")" + _symbol + "(" + _right.toString() + ")";
    } 

    const std::string& getSymbol() const
    {
      return _symbol;
    } 

  private:
    const TypedExpression<InputT>& _left;
    const TypedExpression<Input2T>& _right;
    const OpT& _operator;
    const std::string& _symbol;

  };

  // Operator must be something such as binary_function<InputType, InputType, ReturnType>
  // And default-constructible.
  template <class OperatorT, class InputT = typename __TypeTraits<typename OperatorT::first_argument_type>::actual_type, class Input2T = typename __TypeTraits<typename OperatorT::second_argument_type>::actual_type, class OutputT = typename __TypeTraits<typename OperatorT::result_type>::actual_type>
    class AsymmetricTypedOperator:
      public TypedExpression<OutputT>
  {
  public:
    typedef typename TypeTraits<OutputT>::ReturnType ReturnType;
    typedef typename TypedExpression<InputT>::ReturnType InputType;
    typedef typename TypedExpression<Input2T>::ReturnType Input2Type;

    AsymmetricTypedOperator(const TypedExpression<InputT>& iLeft,
                            const TypedExpression<Input2T>& iRight,
                            const Grammar& iGrammar,
                            const std::string& iSymbol) :
      TypedExpression<OutputT>(iGrammar), _left(iLeft), _right(iRight), _symbol(iSymbol)
    {}


    ReturnType evaluate(IContext& ioContext) const {
      return _operator(_left.evaluate(ioContext), _right.evaluate(ioContext));
    }

    std::string toString() const
    {
      return "(" + _left.toString() + ")" + _symbol + "(" + _right.toString() + ")";
    } 

    const std::string& getSymbol() const
    {
      return _symbol;
    } 

  private:
    const TypedExpression<InputT>& _left;
    const TypedExpression<Input2T>& _right;
    OperatorT _operator;
    const std::string& _symbol;

  };

}}


