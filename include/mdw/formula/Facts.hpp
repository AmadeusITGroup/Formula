#pragma once
#include <string>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/IContext.hpp>

namespace mdw { namespace formula {

  // FunctorT must be such as std::unary_function<ReturnType, RealFactT>
  template <class FunctorT> class Attribute
  {
  public:
    typedef typename __TypeTraits<typename FunctorT::argument_type>::actual_type RealFactT;
    typedef typename __TypeTraits<typename FunctorT::result_type>::actual_type OutputType;
    typedef typename TypeTraits<OutputType>::ReturnType ReturnType;

    class FunctorResolver:
      public TypedExpression<OutputType>
    {
    public:
      FunctorResolver(const TypedExpression<RealFactT>& iObject,
                      const Grammar& iGrammar,
                      const FunctorT& iFunctor,
                      const std::string& iName):
        TypedExpression<OutputType>(iGrammar), _object(iObject), _functor(iFunctor), _name(iName)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        return _functor(_object.evaluate(ioContext));
      }

      std::string toString() const
      {
        return _object.toString() + "." + _name;
      }

    private:
      const TypedExpression<RealFactT>& _object;
      FunctorT _functor;
      const std::string& _name;
    };

    class FunctorInstantiator:
      public UnaryOpInstantiator
    {
      FunctorT _functor;
    public:
      FunctorInstantiator(const FunctorT& iFunctor):
        _functor(iFunctor)
      {}

      virtual Expression& instantiate(ArenaAllocator& ioAllocator,
                                      const Grammar& iGrammar,
                                      const std::string& iName,
                                      const Expression& iFact) const
      {
        const TypedExpression<RealFactT>& anObject = iFact.get<RealFactT>();
        return ioAllocator.create<FunctorResolver>(anObject, iGrammar, _functor, iName);
      }
    };

    static void RegisterMe(ArenaAllocator& ioAllocator,
                           Grammar& ioGrammar,
                           const FunctorT& iFunctor,
                           const std::string& iSymbol)
    {
      ExpressionType aInputType = ioGrammar.registerType<RealFactT>();
      ExpressionType aOutputType = ioGrammar.registerType<OutputType>();
      UnaryOpInstantiator& anInstantiator = ioAllocator.create<FunctorInstantiator>(iFunctor);
      ioGrammar.registerUnaryOperator(aInputType, aOutputType, iSymbol, anInstantiator);
    }

  };

  template <class FunctorT>
    void RegisterAttribute(ArenaAllocator& ioAllocator,
                           Grammar& ioGrammar,
                           const FunctorT& iFunctor,
                           const std::string& iName)
    {
      Attribute<FunctorT>::RegisterMe(ioAllocator, ioGrammar, iFunctor, iName);
    }

  template <class FunctorT>
    TypedExpression<typename Attribute<FunctorT>::OutputType>&
    make_resolver(ArenaAllocator& ioAllocator,
                  const Grammar& iGrammar,
                  const TypedExpression<typename Attribute<FunctorT>::RealFactT>& iObject,
                  const FunctorT& iFunctor,
                  const std::string& iName)
    {
      return ioAllocator.create<typename Attribute<FunctorT>::FunctorResolver>(iObject, iGrammar,
                                                                               iFunctor, iName);
    }

  template <class FunctorT, class HasFunctorT> class OptionalAttribute
  {
  public:
    typedef typename __TypeTraits<typename FunctorT::argument_type>::actual_type RealFactT;
    typedef typename __TypeTraits<typename FunctorT::result_type>::actual_type OutputType;
    typedef typename TypeTraits<OutputType>::ReturnType ReturnType;

    class FunctorResolver:
      public TypedExpression<OutputType>
    {
    public:
      FunctorResolver(const TypedExpression<RealFactT>& iObject,
                      const Grammar& iGrammar,
                      const FunctorT& iFunctor,
                      const HasFunctorT& iHasAttribute,
                      const std::string& iName):
        TypedExpression<OutputType>(iGrammar), _object(iObject),
        _functor(iFunctor), _hasFunctor(iHasAttribute), _name(iName)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        typename TypedExpression<RealFactT>::ReturnType aFact = _object.evaluate(ioContext);
        if (_hasFunctor(aFact))
        {
          return _functor(aFact);
        } else {
          ioContext.setNaN();
          return _invalidValue;
        }
      }

      std::string toString() const
      {
        return _object.toString() + "." + _name;
      }

    private:
      const TypedExpression<RealFactT>& _object;
      FunctorT _functor;
      HasFunctorT _hasFunctor;
      OutputType _invalidValue;
      const std::string& _name;
    };

    class FunctorInstantiator:
      public UnaryOpInstantiator
    {
      FunctorT _functor;
      HasFunctorT _hasFunctor;
    public:
      FunctorInstantiator(const FunctorT& iFunctor, const HasFunctorT& iHasAttribute):
        _functor(iFunctor), _hasFunctor(iHasAttribute)
      {}

      virtual Expression& instantiate(ArenaAllocator& ioAllocator,
                                      const Grammar& iGrammar,
                                      const std::string& iName,
                                      const Expression& iFact) const
      {
        const TypedExpression<RealFactT>& anObject = iFact.get<RealFactT>();
        return ioAllocator.create<FunctorResolver>(anObject, iGrammar, _functor, _hasFunctor, iName);
      }
    };

    static void RegisterMe(ArenaAllocator& ioAllocator,
                           Grammar& ioGrammar,
                           const FunctorT& iFunctor,
                           const HasFunctorT& iHasFunctor,
                           const std::string& iSymbol)
    {
      ExpressionType aInputType = ioGrammar.registerType<RealFactT>();
      ExpressionType aOutputType = ioGrammar.registerType<OutputType>();
      UnaryOpInstantiator& anInstantiator =
        ioAllocator.create<FunctorInstantiator>(iFunctor, iHasFunctor);
      ioGrammar.registerUnaryOperator(aInputType, aOutputType, iSymbol, anInstantiator);
    }

  };

  template <class FunctorT, class HasFunctorT>
    void RegisterOptionalAttribute(ArenaAllocator& ioAllocator,
                                   Grammar& ioGrammar,
                                   const FunctorT& iFunctor,
                                   const HasFunctorT& iHasFunctor,
                                   const std::string& iName)
    {
      OptionalAttribute<FunctorT, HasFunctorT>::RegisterMe(ioAllocator, ioGrammar,
                                                           iFunctor, iHasFunctor, iName);
    }

  template <class FactT> class Fact
  {
  public:
    typedef typename __TypeTraits<FactT>::actual_type OutputType;
    typedef typename TypeTraits<OutputType>::ReturnType ReturnType;

    class DefaultResolver: public TypedExpression<OutputType>
    {
    public:
      DefaultResolver(const Grammar& iGrammar, const std::string& iName):
        TypedExpression<OutputType>(iGrammar), _name(iName)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        return ioContext.getFact<FactT>(_name);
      }

      std::string toString() const
      {
        return "$" + _name;
      }

    private:
      const std::string& _name;
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
        return ioAllocator.create<DefaultResolver>(_grammar, iName);
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

  // In case the user prefers to define its own context with its own getters for facts
  template <class ContextT> class OwnContext
  {
  public:
    // FunctorT must be such as std::unary_function<ReturnType, RealFactT>
    template <class FunctorT> class FactResolver:
      public TypedExpression<typename __TypeTraits<typename FunctorT::result_type>::actual_type>
    {
    public:
      typedef typename __TypeTraits<typename FunctorT::argument_type>::actual_type RealFactT;
      typedef typename __TypeTraits<typename FunctorT::result_type>::actual_type OutputType;
      typedef typename TypeTraits<OutputType>::ReturnType ReturnType;

      FactResolver(const Grammar& iGrammar,
                   const FunctorT& iFunctor,
                   const std::string& iName):
        TypedExpression<OutputType>(iGrammar), _functor(iFunctor), _name(iName)
      {}

      ReturnType evaluate(IContext& ioContext) const
      {
        return _functor(ioContext.get<ContextT>());
      }

      std::string toString() const
      {
        return "$" + _name;
      }

    private:
      FunctorT _functor;
      const std::string& _name;
    };

    template <class FunctorT> class FactInstantiator:
      public mdw::formula::FactInstantiator
    {
    public:
      typedef typename __TypeTraits<typename FunctorT::argument_type>::actual_type RealFactT;
      typedef typename __TypeTraits<typename FunctorT::result_type>::actual_type OutputType;
      typedef typename TypeTraits<OutputType>::ReturnType ReturnType;

      FactInstantiator(const FunctorT& iFunctor):
        _functor(iFunctor)
      {}

      Expression& instantiate(ArenaAllocator& ioAllocator,
                              const Grammar& iGrammar,
                              const std::string& iName) const
      {
        return ioAllocator.create<FactResolver<FunctorT> >(iGrammar, _functor, iName);
      }

    private:
      FunctorT _functor;
    };

    template <class FunctorT>
      static void RegisterFact(ArenaAllocator& ioAllocator,
                               Grammar& ioGrammar,
                               const std::string& iName,
                               const FunctorT& iFunctor)
      {
        FactInstantiator<FunctorT>& anInstantiator =
          ioAllocator.create<FactInstantiator<FunctorT> >(iFunctor);

        ioGrammar.registerFactResolver<typename FactInstantiator<FunctorT>::OutputType>(iName,
                                                                                        anInstantiator);
      }
  };

}}

