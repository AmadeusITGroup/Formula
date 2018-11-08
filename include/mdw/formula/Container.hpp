#pragma once

#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Grammar.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>

namespace mdw { namespace formula {

  class ArenaAllocator;

  class Container: private boost::noncopyable
  {
    ArenaAllocator& _allocator;
    const Grammar& _knownTypes;
    Expression& _topExpression;

  public:
    Container(const std::string& iFormula, const Grammar& ioGrammar);
    ~Container();

    const Expression& getExpression() const;
    const Grammar& getKnownTypes() const;
  };

}}

