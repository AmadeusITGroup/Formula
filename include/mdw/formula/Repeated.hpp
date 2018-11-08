#pragma once
#include <string>
#include <vector>
#include <set>
#include <functional>
#include <algorithm>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Operator.hpp>
#include <mdw/formula/Facts.hpp>
#include <mdw/formula/cache/CachableFacts.hpp>
#include <mdw/formula/ValueException.hpp>
#include <boost/mem_fn.hpp>
#include <boost/foreach.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <mdw/Tracer.hpp>

namespace mdw { namespace formula {

  class IContext;

  template <class T> class ConditionFunctor:
    public std::unary_function<typename TypeTraits<T>::ReturnType, bool>
  {
    const TypedExpression<bool>& _condition;
    IContext& _context;
    const std::string& _variableName;
    mutable TypedFact<const typename __TypeTraits<T>::actual_type> *_fact;
  public:
    explicit ConditionFunctor(const std::string& iVariableName,
                              const TypedExpression<bool>& iExpr,
                              IContext& ioContext):
      _condition(iExpr), _context(ioContext), _variableName(iVariableName), _fact(NULL)
    {}

    const TypedExpression<bool>& getCondition() const
    {
      return _condition;
    }

    bool operator()(const typename TypeTraits<T>::ReturnType ioObject) const
    {
      bool aResult = false;
      if (!_context.isNaN())
      {
        if (_fact)
        {
          _fact->set(ioObject);
        } else {
          _context.setFact(ioObject, _variableName);
          _fact =
            _context.getFactContainer<const typename __TypeTraits<T>::actual_type>(_variableName);
        }
        try {
          aResult = _condition.evaluate(_context);
          if (_context.isNaN())
          {
            aResult = false;
            _context.ignoreNaN();
          }
        } catch (ValueException& iEx) {
          aResult = false;
        }
      }
      return aResult;
    }
  };

  template <class U>
  class Sizer: public std::unary_function<U, typename TypeTraits<int>::ReturnType>
  {
  public:
    static const size_t _Complexity = 10;

    typename TypeTraits<int>::ReturnType operator()(const U& iContainer) const
    {
      typename U::const_iterator anIt = iContainer.begin();
      typename U::const_iterator anEnd = iContainer.end();
      typename TypeTraits<int>::ReturnType aCount = 0;
      while (anIt != anEnd)
      {
        ++aCount;
        ++anIt;
      }
      return aCount;
    }
  };

#define FORM_DECLARE_SIZE(Container) \
  template <class T> class Sizer<Container >: \
    public std::unary_function<const Container&, typename TypeTraits<int>::ReturnType>\
    {\
    public:\
           static const size_t _Complexity = 1; \
      typename TypeTraits<int>::ReturnType operator()(const Container& iContainer) const\
      {\
        return iContainer.size();\
      }\
    }

  FORM_DECLARE_SIZE(std::vector<T>);
  FORM_DECLARE_SIZE(std::list<T>);
  FORM_DECLARE_SIZE(std::set<T>);

  template <> class Sizer<std::string>:
    public std::unary_function<const std::string&, TypeTraits<int>::ReturnType>
    {
    public:
      static const size_t _Complexity = 1;

      TypeTraits<int>::ReturnType operator()(const std::string& iContainer) const
      {
        return iContainer.size();
      }
    };

  template <class U>
  class Empty: public std::unary_function<U, typename TypeTraits<bool>::ReturnType>
  {
  public:
    typename TypeTraits<bool>::ReturnType operator()(const U& iContainer) const
    {
      return iContainer.begin() == iContainer.end();
    }
  };

#define FORM_DECLARE_EMPTY(Container) \
  template <class T> class Empty<Container >:\
    public std::unary_function<const Container&, typename TypeTraits<bool>::ReturnType>\
    {\
    public:\
      typename TypeTraits<bool>::ReturnType operator()(const Container& iContainer) const\
      {\
        return iContainer.empty();\
      }\
    }

  FORM_DECLARE_EMPTY(std::vector<T>);
  FORM_DECLARE_EMPTY(std::list<T>);
  FORM_DECLARE_EMPTY(std::set<T>);

  template <> class Empty<std::string>:
    public std::unary_function<const std::string&, TypeTraits<bool>::ReturnType>
    {
    public:
      TypeTraits<bool>::ReturnType operator()(const std::string& iContainer) const
      {
        return iContainer.empty();
      }
    };

  template <class T, class It> class Filter
  {
    const ConditionFunctor<T>& _predicate;
    It _begin;
    It _end;
  public:
    Filter(const ConditionFunctor<T>& iPred, It& iBegin, It& iEnd):
      _predicate(iPred), _begin(iBegin), _end(iEnd)
    {}

    typedef boost::filter_iterator<ConditionFunctor<T>, It> const_iterator;
    const_iterator begin() const
    {
      return const_iterator(_predicate, _begin, _end);
    }

    const_iterator end() const
    {
      return const_iterator(_predicate, _end, _end);
    }
  };

/*  template <class T, class It>
    struct TypeTraits<Filter<T, It>, void>
  {
    // Return a non-const iterator so that users can call ++ on it
    typedef boost::filter_iterator<P, It>& ReturnType;
    static const char *kTypeAsString;
  };

  template <class P, class It>
    const char *TypeTraits<boost::filter_iterator<P, It>, void>::kTypeAsString =
    typeid(boost::filter_iterator<P, It>).name();*/

  template <class T, class U = std::vector<T> >
    class IterableHelpers
  {
  public:
    typedef typename U::const_iterator Iterator;
    class InExpression: public TypedExpression<bool>
    {
      const TypedExpression<U>& _container;
      const TypedExpression<T>& _object;
    public:
      InExpression(const TypedExpression<U>& iContainer,
                   const TypedExpression<T>& iObject):
        _container(iContainer), _object(iObject)
      {}

      std::string toString() const
      {
        return "(" + _object.toString() + ") in (" + _container.toString() + ")";
      }

      bool evaluate(IContext& ioContext) const
      {
        typename TypedExpression<U>::ReturnType aCont = _container.evaluate(ioContext);
        return std::find(aCont.begin(), aCont.end(), _object.evaluate(ioContext)) != aCont.end();
      }
    };

    class FilterExpression: public TypedExpression<Filter<T, Iterator> >
    {
      const TypedExpression<U>& _container;
      const std::string& _variableName;
      const TypedExpression<bool>& _condition;

    public:
      FilterExpression(const Grammar& iGrammar,
                       const TypedExpression<U>& iContainer,
                       const TypedExpression<bool>& iCondition,
                       const std::string& iVariableName):
        TypedExpression<Filter<T, Iterator> >(iGrammar),
        _container(iContainer), _variableName(iVariableName), _condition(iCondition)
      {}

      std::string toString() const
      {
        return "((" + _container.toString() + ") -> " + _variableName + " ? ("
          + _condition.toString() + "))";
      }

      size_t complexity() const
      {
        return 20;
      }

      typename TypedExpression<Filter<T, Iterator> >::ReturnType evaluate(IContext& ioContext) const
      {
        typename TypedExpression<U>::ReturnType aCont = _container.evaluate(ioContext);
        ConditionFunctor<T>& aCondition =
          ioContext.getAllocator().template create<ConditionFunctor<T> >(_variableName,
                                                                _condition, ioContext);
        Iterator aBegin = aCont.begin();
        Iterator aEnd = aCont.end();
        Filter<T, Iterator> & aResult =
          ioContext.getAllocator().template create<Filter<T, Iterator> >(aCondition, aBegin, aEnd);
        return aResult;
      }
    };

    class CountExpression: public TypedExpression<int>
    {
      const TypedExpression<U>& _container;
    public:
      CountExpression(const TypedExpression<U>& iContainer):
        _container(iContainer)
      {}

      std::string toString() const
      {
        return "(" + _container.toString() + ").count";
      }

      size_t complexity() const
      {
        return Sizer<U>::_Complexity;
      }

      TypedExpression<int>::ReturnType evaluate(IContext& ioContext) const
      {
        Sizer<U> aSizer;
        return aSizer(_container.evaluate(ioContext));
      }
    };

    class EmptyExpression: public TypedExpression<bool>
    {
      const TypedExpression<U>& _container;
    public:
      EmptyExpression(const TypedExpression<U>& iContainer):
        _container(iContainer)
      {}

      std::string toString() const
      {
        return "(" + _container.toString() + ").empty";
      }

      TypedExpression<bool>::ReturnType evaluate(IContext& ioContext) const
      {
        Empty<U> aEmpty;
        return aEmpty(_container.evaluate(ioContext));
      }
    };

    class FilterInstantiator: public ArrowInstantiator
    {
      typename CachableFact<T>::Instantiator _itemInstantiator;
    public:
      FilterInstantiator(const Grammar& iGrammar):
        _itemInstantiator(iGrammar)
      {}

      FactInstantiator *subFact(ArenaAllocator& ioAllocator,
                                const Grammar& iGrammar,
                                const Expression& iLeft,
                                const char *iName)
      {
        return &_itemInstantiator;
      }

      Expression& instantiate(ArenaAllocator& ioAllocator,
                              const Grammar& iGrammar,
                              const std::string& iSymbol,
                              const Expression& iLeft,
                              const Expression& iRight,
                              const std::string& iName) const
      {
        if ((iSymbol == "->") &&
            (iLeft.getType() == iGrammar.findType<U>()) &&
            (iRight.getType() == kExprBool))
        {
          const TypedExpression<U>& aContainer = iLeft.get<U>();
          const TypedExpression<bool>& aCondition = iRight.get<bool>();
          return ioAllocator.create<FilterExpression>(iGrammar, aContainer, aCondition, iName);
        } else {
          throw mdw::UnknownException("Operator is not supported on iterables: " + iSymbol);
        }
      }
    };

    class UnaryInstantiator: public UnaryOpInstantiator
    {
    public:
      Expression& instantiate(ArenaAllocator& ioAllocator,
                              const Grammar& ioGrammar,
                              const std::string& iSymbol,
                              const Expression& iChild) const
      {
        ExpressionType aContType = ioGrammar.findType<U>();
        if ((iSymbol == "count") && (iChild.getType() == aContType))
        {
          const TypedExpression<U>& aContainer = iChild.get<U>();
          return ioAllocator.create<CountExpression>(aContainer);
        } else if ((iSymbol == "empty") && (iChild.getType() == aContType)) {
          return ioAllocator.create<EmptyExpression>(iChild.get<U>());
        } else {
          throw mdw::UnknownException("Unary operator is not supported on iterables: " + iSymbol);
        }
      }
    };

    class BinaryInstantiator: public BinaryOpInstantiator
    {
    public:
      Expression& instantiate(ArenaAllocator& ioAllocator,
                              const Grammar& iGrammar,
                              const std::string& iSymbol,
                              const Expression& iLeft,
                              const Expression& iRight) const
      {
        if (iLeft.getType() == iGrammar.findType<T>())
        {
          if (iRight.getType() == iGrammar.findType<U>())
          {
            if (iSymbol == "in")
            {
              return ioAllocator.create<InExpression>(iRight.get<U>(), iLeft.get<T>());
            }
          }
        }
        throw mdw::UnknownException("Operator " + iSymbol + " is not supported on "
                                    + iLeft.toString());
      }
    };

    static void RegisterOnlyMe(ArenaAllocator& ioAllocator, Grammar& ioGrammar)
    {
      ExpressionType aContType = ioGrammar.registerType<U>();
      ExpressionType aFilter = ioGrammar.registerType<Filter<T, Iterator> >();
      ExpressionType aValueType = ioGrammar.registerType<T>();

      UnaryInstantiator& aUnary = ioAllocator.create<UnaryInstantiator>();
      BinaryInstantiator& aBinary = ioAllocator.create<BinaryInstantiator>();
      FilterInstantiator& aArrow = ioAllocator.create<FilterInstantiator>(ioGrammar);
      ioGrammar.registerBinaryOperator(aValueType, aContType, kExprBool, "in", aBinary);
      ioGrammar.registerUnaryOperator(aContType, kExprBool, "empty", aUnary);
      ioGrammar.registerUnaryOperator(aContType, kExprInt, "count", aUnary);
      ioGrammar.registerArrowOperator(aContType, kExprBool, aFilter, "->", aArrow);
    }
  };

  template <class T, class U = std::vector<T>, int RecursiveLevel = 3>
    class Iterable: public IterableHelpers<T, U>
  {
  public:
    typedef typename IterableHelpers<T, U>::Iterator Iterator;

    static void RegisterMe(ArenaAllocator& ioAllocator, Grammar& ioGrammar)
    {
      IterableHelpers<T, U>::RegisterOnlyMe(ioAllocator, ioGrammar);

      Iterable<T, Filter<T, Iterator>, RecursiveLevel-1>::RegisterMe(ioAllocator, ioGrammar);
    }
  };

  template <class T, class U> class Iterable<T, U, 0>
  {
  public:
    static void RegisterMe(ArenaAllocator& ioAllocator, Grammar& ioGrammar)
    {
      IterableHelpers<T, U>::RegisterOnlyMe(ioAllocator, ioGrammar);
    }
  };

  template <class ObjectT, class ContainerT = std::vector<ObjectT>, class IndexT = typename __TypeTraits<typename ContainerT::size_type>::actual_type>
    struct SquareOp:
      public std::binary_function<typename TypeTraits<ContainerT>::ReturnType, typename TypeTraits<IndexT>::ReturnType, typename TypeTraits<ObjectT>::ReturnType>
  {
    typename TypeTraits<ObjectT>::ReturnType
      operator()(typename TypeTraits<ContainerT>::ReturnType iContainer,
                 typename TypeTraits<IndexT>::ReturnType iIndex) const
      {
        return iContainer[iIndex];
      }
  };

  template <class OpT,
           class ContT = typename __TypeTraits<typename OpT::first_argument_type>::actual_type,
           class ObjT = typename __TypeTraits<typename OpT::result_type>::actual_type,
           class IdxT = typename __TypeTraits<typename OpT::second_argument_type>::actual_type>
             class SquareOperator:
               public TypedExpression<ObjT>
  {
  public:
    typedef typename TypeTraits<ObjT>::ReturnType ReturnType;
    typedef typename TypedExpression<IdxT>::ReturnType IndexType;
    typedef typename TypedExpression<ContT>::ReturnType ContainerType;

    SquareOperator(const TypedExpression<ContT>& iContainer,
                   const TypedExpression<IdxT>& iIndex,
                   const Grammar& iGrammar,
                   const OpT& iOperator = OpT()) :
      TypedExpression<ObjT>(iGrammar), _container(iContainer), _index(iIndex),
      _operator(iOperator)
    {}

    ReturnType evaluate(IContext& ioContext) const {
      return _operator(_container.evaluate(ioContext), _index.evaluate(ioContext));
    }

    std::string toString() const
    {
      return "(" + _container.toString() + ")[" + _index.toString() + "]";
    } 

    const std::string& getSymbol() const
    {
      static const std::string kSquares("[]");
      return kSquares;
    } 

  private:
    const TypedExpression<ContT>& _container;
    const TypedExpression<IdxT>& _index;
    OpT _operator;

  };

  template <class T, class U = std::vector<T>, class IndexT = typename __TypeTraits<typename U::size_type>::actual_type>
    class RandomAccess: public BinaryOpInstantiator
  {
  public:
    Expression& instantiate(ArenaAllocator& ioAllocator,
                            const Grammar& iGrammar,
                            const std::string& iSymbol,
                            const Expression& iLeft,
                            const Expression& iRight) const
    {
      if (iLeft.getType() == iGrammar.findType<U>())
      {
        if (iRight.getType() == iGrammar.findType<IndexT>())
        {
          if (iSymbol == "[]")
          {
            return ioAllocator.create<SquareOperator<SquareOp<T, U, IndexT> > >(iLeft.get<U>(),
                                                                                iRight.get<IndexT>(),
                                                                                iGrammar);
          }
        }
      }
      throw mdw::UnknownException("Operator " + iSymbol + " is not supported on "
                                  + iLeft.toString());
    }

    static void RegisterMe(ArenaAllocator& ioAllocator, Grammar& ioGrammar)
    {
      ExpressionType aContType = ioGrammar.registerType<U>();
      ExpressionType aValueType = ioGrammar.registerType<T>();
      ExpressionType aIndexType = ioGrammar.registerType<IndexT>();

      RandomAccess<T, U, IndexT>& aBinaryInstantiator =
        ioAllocator.create<RandomAccess<T, U, IndexT> >();
      FORMULA_DEBUG("Registered random access for: " << TypeTraits<T>::kTypeAsString
              << " on container " << TypeTraits<U>::kTypeAsString
              << " (" << mdw::lexical_cast<std::string>(aContType)
              << ") with index: " << TypeTraits<IndexT>::kTypeAsString
              << " (" << mdw::lexical_cast<std::string>(aIndexType) << ")");
      ioGrammar.registerBinaryOperator(aContType, aIndexType, aValueType, "[]", aBinaryInstantiator);
    }
  };
}}

