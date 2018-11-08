#pragma once

#include <functional>
#include <mdw/formula/ArenaAllocator.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/IContext.hpp>
#include <mdw/formula/cache/KnownType.hpp>
#include <mdw/Tracer.hpp>
#include <boost/unordered_map.hpp>

namespace mdw { namespace formula {


  template <class OutputType> class UnaryCachedByAddress
  {
  public:
    class Expression: public TypedExpression<OutputType>
    {
      typedef typename TypeTraits<OutputType>::ReturnType ReturnType;
      typedef typename __TypeTraits<ReturnType>::cached_type CachedType;
      typedef boost::unordered_map<const void*, std::pair<bool, CachedType> > Cache;
    public:

      Expression(ExpressionType iType,
                 const TypedExpression<OutputType>& iChild,
                 const FactByAddress& iFact):
        TypedExpression<OutputType>(iType), _child(iChild), _fact(iFact), _latestContextId(-1)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        if (_isWorthCaching && !ioContext.isNaN())
        {
          if (_latestContextId != ioContext.getUniqueId())
          {
            FORMULA_DEBUG("Need to clean up cache due to new IContext");
            _cache.clear();
            _latestContextId = ioContext.getUniqueId();
          }
          const void *aFact = NULL;
          try {
            aFact = _fact.compute(ioContext);
            if (ioContext.isNaN())
            {
              ioContext.ignoreNaN();
              FORMULA_DEBUG("Missing fact while computing fact: "
                      << _fact.getExpression().toString());
              return _child.evaluate(ioContext);
            }
          } catch (...) {
            FORMULA_DEBUG("Exception while computing fact: " << _fact.getExpression().toString());
            return _child.evaluate(ioContext);
          }
          typename Cache::iterator anIt = _cache.find(aFact);
          if (anIt != _cache.end())
          {
            if (anIt->second.first)
            {
              ioContext.setNaN();
            }
            //FORMULA_DEBUG("Used cached value for " << _child.toString());
            return __TypeTraits<ReturnType>::FromCached(anIt->second.second);
          } else {
            ReturnType aResult = _child.evaluate(ioContext);
            //FORMULA_DEBUG("New cached value for " << _child.toString());
            _cache.insert(std::make_pair(aFact, std::make_pair(ioContext.isNaN(),
                                                              __TypeTraits<ReturnType>::ToCached(aResult))));
            return aResult;
          }
        } else {
          FORMULA_DEBUG("Not cached or invalid context for " << _child.toString());
          return _child.evaluate(ioContext);
        }
      }

      size_t complexity() const {
        if (_isWorthCaching)
        {
          return _fact.getExpression().complexity() + 2;
        } else {
          return _child.complexity() + 1;
        }
      }

      /*void enableCache(bool iCache)
      {
        _isWorthCaching = iCache;
      }*/

      std::string toString() const {
        return _child.toString();
      }

      const TypedExpression<OutputType>& getChild() const
      {
        return _child;
      }

    private:
      const TypedExpression<OutputType>& _child;
      const FactByAddress& _fact;
      static const bool _isWorthCaching = true; // Not sure how to set it yet

      mutable Cache _cache;
      mutable int _latestContextId;
    };
  };

}}
