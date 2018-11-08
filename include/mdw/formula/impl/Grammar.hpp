#pragma once

#include <mdw/formula/Traits.hpp>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/cache/Factorizer.hpp>
#include <mdw/UnknownException.hpp>

namespace mdw { namespace formula {

  template <class T> ExpressionType Grammar::registerType()
  {
    ExpressionType aType = findType(TypeTraits<T>::kTypeAsString);
    if (aType != kExprVoid)
    {
      return aType;
    } else {
      _types[TypeTraits<T>::kTypeAsString] = _maxId;
      BOOST_FOREACH(Factorizer *anObserver, _factorizers)
      {
        anObserver->registerType<T>(*this);
      }
      return _maxId++;
    }
  }

  template <class T> ExpressionType Grammar::findType() const
  {
    ExpressionType aType = findType(TypeTraits<T>::kTypeAsString);
    if (aType == kExprVoid)
    {
      throw mdw::UnknownException(std::string("Fact type was not registered: ") +
                                  TypeTraits<T>::kTypeAsString);
    }
    return aType;
  }


}}

