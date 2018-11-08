#pragma once
#include <string>
#include <map>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/IContext.hpp>
#include <mdw/formula/Any.hpp>
#include <mdw/formula/ValueException.hpp>
#include <boost/unordered_map.hpp>

namespace mdw { namespace formula {

  template <class FactT> class CachableFact
  {
  public:
    typedef typename __TypeTraits<FactT>::actual_type OutputType;
    typedef typename TypeTraits<OutputType>::ReturnType ReturnType;

    class DefaultResolver: public TypedExpression<OutputType>
    {
    public:
      DefaultResolver(const Grammar& iGrammar, const std::string& iName):
        TypedExpression<OutputType>(iGrammar), _name(iName),
        _latestContextId(-1), _factContainer(NULL), _constFactContainer(NULL)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        if (ioContext.getUniqueId() != _latestContextId)
        {
          _factContainer = NULL;
          _constFactContainer = NULL;
          _latestContextId = ioContext.getUniqueId();
        }
        if (_factContainer)
        {
          return _factContainer->get();
        } else if (_constFactContainer) {
          return  _constFactContainer->get();
        } else {
          const TypedFact<OutputType> *aCont =
            ioContext.getFactContainer<OutputType>(_name);
          if (aCont)
          {
            _factContainer = aCont;
            return aCont->get();
          } else {
            const TypedFact<const OutputType> *aConstCont =
              ioContext.getFactContainer<const OutputType>(_name);
            if (aConstCont)
            {
              _constFactContainer = aConstCont;
              return aConstCont->get();
            } else {
              throw ValueException(_name.c_str());
            }
          }
        }
      }

      size_t complexity() const
      {
        return 2;
      }

      std::string toString() const
      {
        return "$" + _name;
      }

    private:
      const std::string& _name;
      mutable int _latestContextId;
      mutable const TypedFact<OutputType> *_factContainer;
      mutable const TypedFact<const OutputType> *_constFactContainer;
    };
    
    class Instantiator: public FactInstantiator
    {
    public:
      Instantiator(const Grammar& iGrammar):
        _grammar(iGrammar)
      {}

      Expression& instantiate(ArenaAllocator& ioAllocator,
                              const Grammar& iGrammar,
                              const std::string& iName) const
      {
        DefaultResolver& anExpr = ioAllocator.create<DefaultResolver>(_grammar, iName);
        return anExpr;
      }

    private:
      const Grammar& _grammar;
    };

    static void RegisterMe(ArenaAllocator& ioAllocator, Grammar& ioGrammar, const std::string& iName)
    {
      Instantiator& anInstantiator = ioAllocator.create<Instantiator>(ioGrammar);

      ioGrammar.registerFactResolver<Instantiator>(iName, anInstantiator);
    }
  };

}}

