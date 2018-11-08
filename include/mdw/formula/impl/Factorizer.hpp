#pragma once
#include <mdw/formula/cache/Factorizer.hpp>
#include <mdw/formula/cache/ActualType.hpp>

namespace mdw { namespace formula {

  class Expression;
  class KnownType;

  template <class T> void Factorizer::registerType(const Grammar& iGrammar)
  {
    ExpressionType aType = iGrammar.findType<T>();
    if (aType == kExprVoid)
    {
      throw mdw::UnknownException(std::string("Unknown type: ") + TypeTraits<T>::kTypeAsString);
    }
    if (_types.find(aType) == _types.end())
    {
      // Types only live during the parsing, no need to put them in the ArenaAllocator
      _types[aType] = new ActualType<T>(aType);
    }
  }

}}

