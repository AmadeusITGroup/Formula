#pragma once
#include <mdw/UnknownException.hpp>
#include <mdw/formula/ArenaAllocator.hpp>
#include <boost/unordered_map.hpp>
#include <mdw/formula/Any.hpp>
#include <mdw/formula/Traits.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/IContext.hpp>

namespace mdw { namespace formula {

  template <class T> class Result
  {
    const T *_value;
  public:
    explicit Result(const T *iValue):
      _value(iValue)
    {}

    const typename TypeTraits<T>::ReturnType operator*() const
    {
      if (!_value)
      {
        throw mdw::UnknownException("Please check the validity of results!");
      }
      return *_value;
    }

    const typename TypeTraits<T>::ReturnType *operator->() const
    {
      if (!_value)
      {
        throw mdw::UnknownException("Please check the validity of results!");
      }
      return _value;
    }

    bool operator!() const
    {
      return _value == NULL;
    }
  };

  class ResultCache {
    class ResultKey {
      const void *_expression;
      const void *_fact;

    public:
      ResultKey(const Expression& iExpression, const void *iFact):
        _expression(&iExpression), _fact(iFact)
      {}

      bool operator==(const ResultKey& iOther) const
      {
        return _expression == iOther._expression && _fact == iOther._fact;
      }

      friend std::size_t hash_value(const ResultKey& iKey)
      {
        std::size_t aSeed = 0;
        boost::hash_combine(aSeed, iKey._expression);
        boost::hash_combine(aSeed, iKey._fact);

        return aSeed;
      }
    };

    // Cannot use boost::any because it does copies
    boost::unordered_map<ResultKey, AnyFact*> _generic;
    boost::unordered_map<ResultKey, TypeTraits<int>::ReturnType> _ints;
    boost::unordered_map<ResultKey, TypeTraits<bool>::ReturnType> _bools;

    IContext& _context;

    template <class T>
      typename TypeTraits<T>::ReturnType buildResult(const ResultKey& iKey,
                                                     const TypedExpression<T>& iExpression)
      {
        typedef typename TypeTraits<T>::ReturnType ReturnType;

        boost::unordered_map<ResultKey, AnyFact*>::const_iterator anIt = _generic.find(iKey);
        if (anIt != _generic.end())
        {
          TypedFact<ReturnType> *aResult =
            dynamic_cast<TypedFact<ReturnType> *>(anIt->second);
          if (aResult)
          {
            return *aResult;
          }
        }

        ReturnType aValue = iExpression.evaluate(_context);
        const ReturnType& aCopy =
          CopyIfNeeded<ReturnType>::Copy(aValue, _context.getAllocator());

        TypedFact<ReturnType>& aNew =
          _context.getAllocator().create<TypedFact<ReturnType> >(aValue);
        _generic[iKey] = &aNew;

        return aValue;
      }

  public:
    ResultCache(IContext& ioContext):
      _context(ioContext)
    {}

    void clean();

    template <class T, class FactT>
      typename TypeTraits<T>::ReturnType findUnary(const TypedExpression<T>& iExpression,
                                                   const FactT& iFact)
    {
      ResultKey aKey(iExpression, &iFact);
      return buildResult<T>(aKey, iExpression);
    }

  };
  
  template <>
    TypeTraits<int>::ReturnType
    ResultCache::buildResult<int>(const ResultKey& iKey, const TypedExpression<int>& iExpression)
    {
      typedef TypeTraits<int>::ReturnType ReturnType;

      boost::unordered_map<ResultKey, ReturnType>::const_iterator anIt = _ints.find(iKey);
      if (anIt != _ints.end())
      {
        return anIt->second;
      }
      ReturnType aValue = iExpression.evaluate(_context);
      _ints[iKey] = aValue;

      return aValue;
    }

  template <>
    TypeTraits<bool>::ReturnType
    ResultCache::buildResult<bool>(const ResultKey& iKey, const TypedExpression<bool>& iExpression)
    {
      typedef TypeTraits<bool>::ReturnType ReturnType;

      boost::unordered_map<ResultKey, ReturnType>::const_iterator anIt = _bools.find(iKey);
      if (anIt != _bools.end())
      {
        return anIt->second;
      }
      ReturnType aValue = iExpression.evaluate(_context);
      _bools[iKey] = aValue;

      return aValue;
    }

  template <class T> class UnaryCacheLookup: public TypedExpression<T>
  {
    const TypedExpression<T>& _child;
    const Expression& _fact;
  public:
    UnaryCacheLookup(const TypedExpression<T>& iChild, const Expression& iFact):
      _child(iChild), _fact(iFact)
    {}

    std::string toString() const
    {
      return _child.toString();
    }

    typename TypeTraits<T>::ReturnType evaluate(IContext& ioContext) const
    {
      return ioContext.getResults().findUnary<T>(_child, _fact);
    }
  };

}}

