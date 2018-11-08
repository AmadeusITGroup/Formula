#pragma once
#include <mdw/UnknownException.hpp>
#include <mdw/formula/ArenaAllocator.hpp>
#include <mdw/formula/Any.hpp>
#include <boost/noncopyable.hpp>
#include <map>

namespace mdw { namespace formula {

  class ResultCache;

  class IContext: private boost::noncopyable
  {
    ArenaAllocator& _allocator;
    ResultCache *_cache;
    int _uniqueId;
    bool _ownsAllocator;
    bool _invalidExpression;

    // Cannot use boost::any because it does copies
    std::map<std::string, AnyFact*> _knownFacts;

    // Yeah yeah, we'll have issues if we go multithread...
    // Just do an std::atomic increment that day because we'll be in C++11 by that time!
    static int LatestUniqueId;

  public:
    explicit IContext(ArenaAllocator& ioAllocator);

    IContext();

    virtual ~IContext();

    const ArenaAllocator& getAllocator() const;

    ArenaAllocator& getAllocator();

    void clean();
    bool isNaN() const
    {
      return _invalidExpression;
    }

    void setNaN()
    {
      _invalidExpression = true;
    }

    void ignoreNaN()
    {
      _invalidExpression = false;
    }

    int getUniqueId() const
    {
      return _uniqueId;
    }

    template <class T> T& get()
    {
      T *aRealContext = dynamic_cast<T*>(this);
      if (aRealContext)
      {
        return *aRealContext;
      } else {
        throw mdw::UnknownException("Context is not of expected type");
      }
    }

    template <class T> const T& get() const
    {
      const T *aRealContext = dynamic_cast<const T*>(this);
      if (aRealContext)
      {
        return *aRealContext;
      } else {
        throw mdw::UnknownException("Context is not of expected type");
      }
    }

    template <class T> void setFact(T& iFact, const std::string& iName)
    {
      std::map<std::string, AnyFact*>::iterator anIt = _knownFacts.find(iName);
      if (anIt == _knownFacts.end())
      {
        _knownFacts[iName] = &getAllocator().template create<TypedFact<T> >(iFact);
      } else {
        TypedFact<T> *aFact = dynamic_cast<TypedFact<T>*>(anIt->second);
        if (aFact)
        {
          aFact->set(iFact);
        } else {
          throw mdw::UnknownException("Fact has changed type or constness when setting it?! " + iName);
        }
      }
    }

    template <class T> const T& getFact(const std::string& iName) const
    {
      std::map<std::string, AnyFact*>::const_iterator anIt = _knownFacts.find(iName);
      if (anIt == _knownFacts.end())
      {
        throw mdw::UnknownException("Fact has not been set: " + iName);
      } else {
        const TypedFact<T> *aFact = dynamic_cast<const TypedFact<T>*>(anIt->second);
        if (aFact)
        {
          return aFact->get();
        } else {
          const TypedFact<const T> *aConstFact =
            dynamic_cast<const TypedFact<const T>*>(anIt->second);
          if (aConstFact)
          {
            return aConstFact->get();
          } else {
            throw mdw::UnknownException("Fact has changed types when getting it?! " + iName);
          }
        }
      }
    }

    template <class T> T& getMutableFact(const std::string& iName)
    {
      std::map<std::string, AnyFact*>::iterator anIt = _knownFacts.find(iName);
      if (anIt == _knownFacts.end())
      {
        throw mdw::UnknownException("Fact has not been set: " + iName);
      } else {
        TypedFact<T> *aFact = dynamic_cast<TypedFact<T>*>(anIt->second);
        if (aFact)
        {
          return aFact->get();
        } else {
          throw mdw::UnknownException("Fact has changed types or is const?! " + iName);
        }
      }
    }

    template <class T> TypedFact<T> *getFactContainer(const std::string& iName)
    {
      std::map<std::string, AnyFact*>::iterator anIt = _knownFacts.find(iName);
      if (anIt == _knownFacts.end())
      {
        return NULL;
      } else {
        TypedFact<T> *aFact = dynamic_cast<TypedFact<T>*>(anIt->second);
        if (aFact)
        {
          return aFact;
        } else {
          return NULL;
        }
      }
    }

  };
  

}}

