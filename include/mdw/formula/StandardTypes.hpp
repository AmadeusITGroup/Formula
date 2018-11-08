#pragma once
#include <string>
#include <map>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/IContext.hpp>

namespace mdw { namespace formula {

  class StandardUnary: public UnaryOpInstantiator
  {
  public:
    Expression& instantiate(ArenaAllocator& ioAllocator,
                            const Grammar& iGrammar,
                            const std::string& iSymbol,
                            const Expression& iChild) const;

    static void RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator);
  };

  class StandardBinary: public BinaryOpInstantiator
  {
  public:
    Expression& instantiate(ArenaAllocator& ioAllocator,
                            const Grammar& iGrammar,
                            const std::string& iSymbol,
                            const Expression& iLeft,
                            const Expression& iRight) const;

    static void RegisterMe(Grammar& ioGrammar, ArenaAllocator& ioAllocator);
  };

}}

